// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/MGameMode.h"

AMGameMode::AMGameMode()
{
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Multiplayer/GameFramework/PC_Base"));
	if (PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}

void AMGameMode::SetSpawnItems(FItemTypeInfo TypeInfo, FVector Location)
{
	if (AMSpawnItemManager* itemManager = Cast<AMSpawnItemManager>(ItemManager))
	{
		itemManager->SpawnItem(TypeInfo, Location);
	}
}

void AMGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(CaptureBaseManagerClass))
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;
		GetWorld()->SpawnActor<AActor>(CaptureBaseManagerClass, SpawnInfo);
	}

	if (IsValid(SpawnBotManagerClass))
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;
		GetWorld()->SpawnActor<AActor>(SpawnBotManagerClass, SpawnInfo);
	}

	if (IsValid(ItemManagerClass))
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;
		ItemManager = GetWorld()->SpawnActor<AActor>(ItemManagerClass, SpawnInfo);
	}
}