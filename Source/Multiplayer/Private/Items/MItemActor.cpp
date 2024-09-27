// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/MItemActor.h"
#include "Character/MBaseCharacter.h"

// Sets default values
AMItemActor::AMItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

	SetReplicates(true);
	SetReplicateMovement(true);
}

void AMItemActor::GetStaticMesh()
{
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMItemActor::OnUpdatedComponentOverlapBegin);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AMItemActor::OnUpdatedComponentOverlapEnd);
	FStreamableManager& streamableManager = UAssetManager::GetStreamableManager();
	streamableManager.RequestAsyncLoad(StaticMeshAsset.ToSoftObjectPath(), [this]() {
		SetStaticMesh(StaticMeshAsset.Get());
		});
}

void AMItemActor::InitializeItemData(FItemData* NewItemData)
{
	TypeInfo = NewItemData->TypeInfo;
	Description = NewItemData->Description;
	StaticMeshAsset = NewItemData->Asset.StaticMesh;
	SetScale(NewItemData->Asset.MeshScale);
	GetStaticMesh();
}

void AMItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMItemActor, StaticMesh);
	DOREPLIFETIME(AMItemActor, ScaleVector);
}

void AMItemActor::OnUpdatedComponentOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMBaseCharacter* player = Cast<AMBaseCharacter>(Other))
	{
		if (InsertPlayerArray.Find(player) == INDEX_NONE)
		{
			InsertPlayerArray.Add(player);

			FString text = "To pick up item press " + player->GetButtonTextForInformWidget("PickUp") + " button";
			ShowInfoText(player->GetPlayerName(), text);
		}
	}
}

void AMItemActor::OnUpdatedComponentOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMBaseCharacter* player = Cast<AMBaseCharacter>(Other))
	{
		if (InsertPlayerArray.Find(player) != INDEX_NONE)
		{
			FString text = "";
			ShowInfoText(player->GetPlayerName(), text);
			InsertPlayerArray.Remove(player);
		}
	}
}

void AMItemActor::OnRep_StaticMesh()
{
	MeshComponent->SetStaticMesh(const_cast<UStaticMesh*>(StaticMesh));
	CollisionComponent->SetSphereRadius(MeshComponent->GetStaticMesh()->GetBounds().SphereRadius * 4);
}

void AMItemActor::OnRep_ScaleVector()
{
	SetActorScale3D(ScaleVector);
}

void AMItemActor::ShowInfoText_Implementation(const FString& PlayerName, const FString& Text)
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
						playerController->ShowInfoForCharacter(Text);

						playerController->SetPickUpItemToInventory(Text.IsEmpty() ? FItemTypeInfo() : TypeInfo, this);
					}
				}
			}
		}
	}
}

void AMItemActor::SetStaticMesh_Implementation(const UStaticMesh* NewStaticMesh)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		MeshComponent->SetStaticMesh(const_cast<UStaticMesh*>(NewStaticMesh));
		CollisionComponent->SetSphereRadius(MeshComponent->GetStaticMesh()->GetBounds().SphereRadius * 4);
	}

	StaticMesh = const_cast<UStaticMesh*>(NewStaticMesh);
}

void AMItemActor::SetScale_Implementation(const FVector& NewScale)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		SetActorScale3D(NewScale);
	}

	ScaleVector = NewScale;
}
