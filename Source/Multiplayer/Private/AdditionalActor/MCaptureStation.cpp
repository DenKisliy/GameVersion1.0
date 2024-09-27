// Fill out your copyright notice in the Description page of Project Settings.


#include "AdditionalActor/MCaptureStation.h"

// Sets default values
AMCaptureStation::AMCaptureStation()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	CollisionComponent->SetShouldUpdatePhysicsVolume(true);
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->bDynamicObstacle = true;
	RootComponent = CollisionComponent;

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMCaptureStation::OnUpdatedComponentOverlapBegin);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AMCaptureStation::OnUpdatedComponentOverlapEnd);

	PlatformComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformComponent"));
	PlatformComponent->SetupAttachment(CollisionComponent);

	StatueComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueComponent"));
	StatueComponent->SetupAttachment(CollisionComponent);

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));
	ParticleComponent->AttachToComponent(CollisionComponent, FAttachmentTransformRules::KeepRelativeTransform);

	NavModifierComponent = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifierComponent"));
	NavModifierComponent->SetIsReplicated(true);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void AMCaptureStation::BeginPlay()
{
	Super::BeginPlay();

	if (PlatformComponent->GetStaticMesh())
	{
		CollisionComponent->SetSphereRadius(PlatformComponent->GetStaticMesh()->GetBounds().SphereRadius);
		NavModifierComponent->FailsafeExtent = FVector(PlatformComponent->GetStaticMesh()->GetBounds().SphereRadius);
	}
}

void AMCaptureStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCaptureStation, NewStatueMesh);
}

void AMCaptureStation::OnUpdatedComponentOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMBaseCharacter* hitActor = Cast<AMBaseCharacter>(Other))
	{
		FVector center = GetActorLocation();
		FVector hitActorLocation = hitActor->GetActorLocation();

		if (InsertPlayerArray.Find(hitActor) == INDEX_NONE && FMath::Sqrt(FMath::Pow(center.X - hitActorLocation.X, 2) + FMath::Pow(center.Y - hitActorLocation.Y, 2)) <= CollisionComponent->GetScaledSphereRadius() + hitActor->GetCapsuleComponent()->GetScaledCapsuleRadius())
		{
			if (!bCapture)
			{
				if (AMPlayerState* playerState = Cast<AMPlayerState>(hitActor->GetPlayerState()))
				{
					hitActor->InsertInSaveZoneDelegate.Broadcast(hitActor);
					playerState->SetPlayerInSaveZone(true);
					ActivateGameplayEffectForCharacter(hitActor, true);
				}
			}
			else
			{
				InsertPlayerArray.Add(hitActor);
				ChangeCountOfCapturePlayerDelegate.Broadcast(InsertPlayerArray.Num());
			}
		}
	}
}

void AMCaptureStation::OnUpdatedComponentOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMBaseCharacter* hitActor = Cast<AMBaseCharacter>(Other))
	{
		if (!bCapture)
		{
			if (AMPlayerState* playerState = Cast<AMPlayerState>(hitActor->GetPlayerState()))
			{
				playerState->SetPlayerInSaveZone(false);
				ActivateGameplayEffectForCharacter(hitActor, false);
			}
		}
		else
		{
			if (InsertPlayerArray.Find(hitActor) != INDEX_NONE)
			{
				InsertPlayerArray.Remove(hitActor);
				ChangeCountOfCapturePlayerDelegate.Broadcast(InsertPlayerArray.Num() > 0 ? 0 : InsertPlayerArray.Num());
			}
		}
	}
}

void AMCaptureStation::ActivateGameplayEffectForCharacter(AMBaseCharacter* Character, bool bValue)
{
	if (IsValid(GameplayEffect))
	{
		if (bValue)
		{
			FGameplayEffectContextHandle EffectContext = Character->GetAbilitySystemComponent()->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				Character->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
		else
		{
			Character->GetAbilitySystemComponent()->RemoveActiveGameplayEffectBySourceEffect(GameplayEffect, Character->GetAbilitySystemComponent());
		}
	}
}

void AMCaptureStation::OnRep_NewStatueMesh()
{
	StatueComponent->SetStaticMesh(NewStatueMesh);
}

void AMCaptureStation::CaptureFinish()
{
	bCapture = false;
	if (IsValid(RestoredStatueMesh))
	{
		NewStatueMesh = RestoredStatueMesh;

		if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
		{
			NavModifierComponent->SetAreaClass(UNavArea_Null::StaticClass());
			StatueComponent->SetStaticMesh(NewStatueMesh);
		}

		AddItems();
	}
}

void AMCaptureStation::AddItems_Implementation()
{
	if (GetWorld()->GetGameState())
	{
		TArray<TObjectPtr<APlayerState>> playerArray = GetWorld()->GetGameState()->PlayerArray;
		for (int i = 0; i < playerArray.Num(); i++)
		{
			if (APlayerState* playerState = playerArray[i].Get())
			{
				for (AMBaseCharacter* player : InsertPlayerArray)
				{
					if (player->GetPlayerName() == playerState->GetPlayerName())
					{
						if (AMPlayerController* playerController = Cast<AMPlayerController>(playerState->GetPlayerController()))
						{
							for (FItemTypeInfo item : RewardItemsArray)
							{
								playerController->AddItemToCharacter(item);
							}
						}
					}
				}
			}
		}
	}
	InsertPlayerArray.Empty();
}
