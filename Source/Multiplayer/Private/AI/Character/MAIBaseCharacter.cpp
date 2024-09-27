// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Character/MAIBaseCharacter.h"

// Sets default values
AMAIBaseCharacter::AMAIBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	Attributes = CreateDefaultSubobject<UMAttributeSet>(TEXT("Attributes"));

	AIWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(FName("AI Widget"));
	AIWidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	SetReplicates(true);
	SetReplicateMovement(true);
}

void AMAIBaseCharacter::FindNextPointAndIndex(FVector& NextPoint, int& Index)
{
	if (PatrolPath)
	{
		PatrolPath->FindNextPointAndIndex(NextPoint, Index);
	}
}

// Called when the game starts or when spawned
void AMAIBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitPlayer();
}

void AMAIBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateAIWidgetComponent();
}

void AMAIBaseCharacter::StartMove(AMPatrolPath* NewPatrolPath)
{
	PatrolPath = NewPatrolPath;
	StartMoveDelegate.Broadcast();
}

void AMAIBaseCharacter::InitPlayer()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAttributes();
		InitializeAbilities();
	}
}

void AMAIBaseCharacter::InitializeAttributes()
{
	if (AbilitySystemComponent && StartupEffects.Num() > 0)
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

		if (UMAttributeWidget* widget = Cast<UMAttributeWidget>(AIWidgetComponent->GetWidget()))
		{
			widget->SetColor(FLinearColor::Red);
			widget->SetPercent(1.0f);
		}
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetHealthAttribute()).AddUObject(this, &AMAIBaseCharacter::OnHealthUpdated);
	}
}

void AMAIBaseCharacter::InitializeAbilities()
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

void AMAIBaseCharacter::OnHealthUpdated(const FOnAttributeChangeData& Data)
{
	if (UMAttributeWidget* widget = Cast<UMAttributeWidget>(AIWidgetComponent->GetWidget()))
	{
		widget->SetPercent(Data.NewValue / Attributes->GetMaxHealth());
	}

	if (Data.NewValue == 0)
	{
		AIDeathDelegate.Broadcast(GetActorLocation());
		Destroy();
	}
}

void AMAIBaseCharacter::RotateAIWidgetComponent()
{
	FRotator rotator = UKismetMathLibrary::FindLookAtRotation(AIWidgetComponent->GetComponentLocation(),
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation());
	AIWidgetComponent->SetWorldRotation(rotator);
}

void AMAIBaseCharacter::MulticastPlayMontage_Implementation(UAnimMontage* AnimMontage)
{
	PlayAnimMontage(AnimMontage);
}
