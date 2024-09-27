// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MBaseCharacter.h"

// Sets default values
AMBaseCharacter::AMBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 110.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	//400
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SpawnArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnArrow"));
	if (SpawnArrowComponent)
	{
		SpawnArrowComponent->ArrowColor = FColor(255, 0, 0);
		SpawnArrowComponent->bTreatAsASprite = true;
		SpawnArrowComponent->bIsScreenSizeScaled = true;
		SpawnArrowComponent->SetupAttachment(RootComponent);
	}

	PlayerTagComponent = CreateDefaultSubobject<UWidgetComponent>(FName("Player Tag Widget"));
	PlayerTagComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	Attributes = CreateDefaultSubobject<UMAttributeSet>(TEXT("Attributes"));

	InventoryComponent = CreateDefaultSubobject<UMInventoryComponent>(TEXT("Inventory"));
	
	SetReplicates(true);
	SetReplicateMovement(true);
}

FString AMBaseCharacter::GetPlayerName()
{
	FString Name;

	if (UMPlayerInfoWidget* playerTag = Cast<UMPlayerInfoWidget>(PlayerTagComponent->GetWidget()))
	{
		Name = playerTag->GetPlayerName();
	}

	return Name;
}

FString AMBaseCharacter::GetButtonTextForInformWidget(FString ButtonName)
{
	FString result = "";
	TArray<FString> Components;
	const FString DefaultInputPath = FString::Printf(TEXT("%sDefaultInput.ini"), *FPaths::SourceConfigDir());
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), Components, DefaultInputPath);
	
	FString findStr = *Components.FindByPredicate([](const FString& InItem)
		{
			return InItem.Contains("PickUp");
		});

	if (!findStr.IsEmpty())
	{
		for (int32 j = findStr.Find("Key=") + 4; j < findStr.Len() - 1; j++)
		{
			result = result + findStr[j];
		}
	}

	return result;
}

// Called when the game starts or when spawned
void AMBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
	{
		if (PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}

			SetLocalPlayerTag();
			BindAllDelegates();
		}
	}
	InitPlayer();
}

// Called every frame
void AMBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotatePlayerNameWidget();
}

// Called to bind functionality to input
void AMBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMBaseCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMBaseCharacter::Look);

		if (AbilitySystemComponent)
		{
			FTopLevelAssetPath AbilityEnumAssetPath = FTopLevelAssetPath(FName("/Script/Multiplayer"), FName("EGDAbilityInputID"));
			AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("Confirm"),
				FString("Cancel"), AbilityEnumAssetPath, static_cast<int32>(EGDAbilityInputID::Confirm), static_cast<int32>(EGDAbilityInputID::Cancel)));
		}

		FInputActionBinding PickUp("PickUp", IE_Pressed);

		PickUp.ActionDelegate.GetDelegateForManualSet().BindLambda([this]()
			{
				InventoryComponent->AddPickUpItemToInventory();
			});

		PlayerInputComponent->AddActionBinding(PickUp);
	}
}

void AMBaseCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMBaseCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMBaseCharacter::OnRep_PlayerNameWidget()
{
	SetPlayerTagName(PlayerNameWidget);
}

void AMBaseCharacter::SetPlayerName_Implementation(const FString& NewPlayerName)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		SetPlayerTagName(NewPlayerName);
	}
	
	PlayerNameWidget = NewPlayerName;
}

void AMBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMBaseCharacter, PlayerNameWidget);
	DOREPLIFETIME(AMBaseCharacter, PlayerScaleVector);
	DOREPLIFETIME(AMBaseCharacter, WalkSpeed);
}

void AMBaseCharacter::SetLocalPlayerTag()
{
	if (GetPlayerState())
	{
		if (AMPlayerState* playerState = Cast<AMPlayerState>(GetPlayerState()))
		{
			playerState->UpdatePlayerState();

			if (GetPlayerState()->GetPlayerName() != "")
			{
				SetPlayerName(GetPlayerState()->GetPlayerName());
			}
		}
		else
		{
			StartTimer();
		}

	}
	else
	{
		StartTimer();
	}
}

void AMBaseCharacter::SetPlayerTagName(FString PlayerName)
{
	UMPlayerInfoWidget* playerTag = Cast<UMPlayerInfoWidget>(PlayerTagComponent->GetWidget());

	if (playerTag)
	{
		playerTag->SetPlayerName(PlayerName);
	}
	else
	{
		StartPlayerWidgetTimer(PlayerName);
	}
}

void AMBaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}

			SetLocalPlayerTag();
			BindAllDelegates();
		}
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	InitPlayer();

}

void AMBaseCharacter::OnRep_PlayerScaleVector()
{
	GetCapsuleComponent()->SetRelativeScale3D(PlayerScaleVector);
}

void AMBaseCharacter::OnRep_WalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMBaseCharacter::SetPlayerSpeed_Implementation(const float& NewSpeed)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}

	WalkSpeed = NewSpeed;
}

AMPlayerHUD* AMBaseCharacter::GetHUD()
{
	if (AMPlayerController* playerController = Cast<AMPlayerController>(Controller))
	{
		if (AMPlayerHUD* hud = Cast<AMPlayerHUD>(playerController->GetHUD()))
		{
			return hud;
		}
	}
	return nullptr;
}

void AMBaseCharacter::SetPlayerScale_Implementation(const FVector& NewPlayerScale)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		GetCapsuleComponent()->SetRelativeScale3D(NewPlayerScale);
	}

	PlayerScaleVector = NewPlayerScale;
}

void AMBaseCharacter::StartTimer()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMBaseCharacter::SetLocalPlayerTag, 1.0f, false);
}

void AMBaseCharacter::StartPlayerWidgetTimer(FString PlayerName)
{
	FTimerDelegate TimerDel;
	FTimerHandle TimerHandle;
	TimerDel.BindUFunction(this, FName("SetPlayerTagName"), PlayerName);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 1.f, false);
}

void AMBaseCharacter::RotatePlayerNameWidget()
{
	if (!IsLocallyControlled())
	{
		FRotator rotator = UKismetMathLibrary::FindLookAtRotation(PlayerTagComponent->GetComponentLocation(),
			UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation());
		PlayerTagComponent->SetWorldRotation(rotator);
	}
}

void AMBaseCharacter::InitializeAttributes()
{
	if (AbilitySystemComponent && StartupEffects.Num()>0)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		
		for (TSubclassOf<UGameplayEffect> GameplayEffect : StartupEffects)
		{
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetHealthAttribute()).AddUObject(this, &AMBaseCharacter::OnHealthUpdated);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetManaAttribute()).AddUObject(this, &AMBaseCharacter::OnManaUpdated);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetStaminaAttribute()).AddUObject(this, &AMBaseCharacter::OnStaminaUpdated);
	}
}

void AMBaseCharacter::InitializeAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		for (TSubclassOf<UMBaseAbility>& Ability : DefaultAbilities)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this));
		}
	}

}

void AMBaseCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Attributes->GetHealth() == 0)
	{
		if (APlayerController* playerController = Cast<APlayerController>(Controller))
		{
			playerController->SetIgnoreMoveInput(true);
			AbilitySystemComponent->ClearAllAbilities();
		}
	}
}

void AMBaseCharacter::BindAllDelegates()
{
	if (AMPlayerController* playerController = Cast<AMPlayerController>(Controller))
	{
		playerController->AddItemToInventoryDelegate.AddDynamic(this, &AMBaseCharacter::OnAddItemToInventory);
		playerController->PickUpItemToInventoryDelegate.AddDynamic(this, &AMBaseCharacter::PickUpItemToInventory);

		playerController->SetPlayerHUD();
	}

	InventoryComponent->UseItemDelegate.AddDynamic(this, &AMBaseCharacter::UseItemGameplayEffect);
}

void AMBaseCharacter::OnAddItemToInventory(FItemTypeInfo ItemType)
{
	InventoryComponent->IsAddItemToInventory(ItemType);
}

void AMBaseCharacter::InitPlayer()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAttributes();
		InitializeAbilities();
	}
}

void AMBaseCharacter::PickUpItemToInventory(FItemTypeInfo ItemType, const AActor* Item)
{
	InventoryComponent->SetPickUpItem(ItemType, Item);
}

void AMBaseCharacter::OnHealthUpdated(const FOnAttributeChangeData& Data)
{
	if (AMPlayerController* playerController = Cast<AMPlayerController>(Controller))
	{
		if (AMPlayerHUD* hud = Cast<AMPlayerHUD>(playerController->GetHUD()))
		{
			hud->SetValueForAttribute(EAttributeType::Health, Data.NewValue / Attributes->GetMaxHealth());

			if (Data.NewValue == 0 && IsValid(DeathMontage))
			{
				playerController->CharacterDeath();
			}
		}
	}
}

void AMBaseCharacter::OnManaUpdated(const FOnAttributeChangeData& Data)
{
	if (APlayerController* playerController = Cast<APlayerController>(Controller))
	{
		if (AMPlayerHUD* hud = Cast<AMPlayerHUD>(playerController->GetHUD()))
		{
			hud->SetValueForAttribute(EAttributeType::Mana, Data.NewValue / Attributes->GetMaxMana());
		}
	}
}

void AMBaseCharacter::OnStaminaUpdated(const FOnAttributeChangeData& Data)
{
	if (APlayerController* playerController = Cast<APlayerController>(Controller))
	{
		if (AMPlayerHUD* hud = Cast<AMPlayerHUD>(playerController->GetHUD()))
		{
			hud->SetValueForAttribute(EAttributeType::Stamina, Data.NewValue / Attributes->GetMaxStamina());
		}
	}
}

void AMBaseCharacter::UseItemGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect)
{
	if (GameplayEffect)
	{
		FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AMBaseCharacter::ServerAnimMontage_Implementation(UAnimMontage* Montage)
{
	MulticastAnimMontage(Montage);
}

void AMBaseCharacter::MulticastAnimMontage_Implementation(UAnimMontage* Montage)
{
	if (Montage == DeathMontage)
	{
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
	}
	else
	{
		if (UAnimInstance* animInstance = GetMesh()->GetAnimInstance())
		{
			animInstance->Montage_Play(Montage, 1.0f);

			/*if (!animInstance->OnMontageEnded.IsBound())
			{
				animInstance->OnMontageEnded.AddDynamic(this, &AMBaseCharacter::OnMontageEnded);
			}*/
		}
	}
}
