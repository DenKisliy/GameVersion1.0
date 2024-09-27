// Fill out your copyright notice in the Description page of Project Settings.


#include "AdditionalActor/MResurrectionFlower.h"

// Sets default values
AMResurrectionFlower::AMResurrectionFlower()
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

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));
	ParticleComponent->AttachToComponent(CollisionComponent, FAttachmentTransformRules::KeepRelativeTransform);

	SetReplicates(true);
}

void AMResurrectionFlower::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMResurrectionFlower::OnUpdatedComponentOverlapBegin);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AMResurrectionFlower::OnUpdatedComponentOverlapEnd);
}

void AMResurrectionFlower::OnUpdatedComponentOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Player)
	{
		Player = Cast<AMBaseCharacter>(Other);
		if (Player)
		{
			IsHaveResurrectionItemInInventory(Player->GetPlayerName());
		}
		else
		{
			Player = nullptr;
		}
	}
}

void AMResurrectionFlower::OnUpdatedComponentOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Player)
	{
		if (AMGameState* gameState = Cast<AMGameState>(GetWorld()->GetGameState()))
		{
			gameState->StopResurrectionTimer();
			gameState->AdditionalTimerDelegate.Clear();
		}
		Player = nullptr;
	}
}

void AMResurrectionFlower::OnFinishTimer(ETypeOfAdditionalTimer TypeOfAdditionalTimer)
{
	if (AMPlayerController* playerController = Cast<AMPlayerController>(Controller))
	{
		RemoveResurrectionItemFromInventory(Player->GetPlayerName());
		playerController->ResurrectionCharacter();
	}
}

void AMResurrectionFlower::IsHaveResurrectionItemInInventory_Implementation(const FString& PlayerName)
{
	if (GetWorld()->GetGameState())
	{
		TArray<TObjectPtr<APlayerState>> playerArray = GetWorld()->GetGameState()->PlayerArray;
		for (int i = 0; i < playerArray.Num(); i++)
		{
			if (APlayerState* playerState = playerArray[i].Get())
			{
				if (playerState->GetPlayerName() == PlayerName)
				{
					if (AMPlayerController* playerController = Cast<AMPlayerController>(playerState->GetPlayerController()))
					{
						if (AMBaseCharacter* player = Cast<AMBaseCharacter>(playerController->GetPawn()))
						{
							if (player->InventoryComponent->IsHaveItem(ItemForResurrectionInfo))
							{
								if (AMGameState* gameState = Cast<AMGameState>(GetWorld()->GetGameState()))
								{
									gameState->StartResurrectionTimer();
									gameState->AdditionalTimerDelegate.Clear();
									gameState->AdditionalTimerDelegate.AddDynamic(this, &AMResurrectionFlower::OnFinishTimer);
								}
							}
							else
							{
								Player = nullptr;
							}
						}
					}
				}
			}
		}
	}
}

void  AMResurrectionFlower::RemoveResurrectionItemFromInventory_Implementation(const FString& PlayerName)
{
	if (GetWorld()->GetGameState())
	{
		TArray<TObjectPtr<APlayerState>> playerArray = GetWorld()->GetGameState()->PlayerArray;
		for (int i = 0; i < playerArray.Num(); i++)
		{
			if (APlayerState* playerState = playerArray[i].Get())
			{
				if (playerState->GetPlayerName() == PlayerName)
				{
					if (AMPlayerController* playerController = Cast<AMPlayerController>(playerState->GetPlayerController()))
					{
						if (AMBaseCharacter* player = Cast<AMBaseCharacter>(playerController->GetPawn()))
						{
							player->InventoryComponent->UseItem(ItemForResurrectionInfo);
						}
					}
				}
			}
		}
	}
}
