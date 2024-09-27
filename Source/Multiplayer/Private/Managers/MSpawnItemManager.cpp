// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/MSpawnItemManager.h"

// Sets default values
AMSpawnItemManager::AMSpawnItemManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AMSpawnItemManager::SpawnItem(FItemTypeInfo ItemType, FVector SpawnLocation)
{
	if (IsValid(ItemStatic))
	{
		if (FItemData* itemData = GetDataFromDataTable(ItemType))
		{
			if (AMItemActor* item = GetWorld()->SpawnActor<AMItemActor>(ItemStatic, SpawnLocation, FRotator(0), FActorSpawnParameters()))
			{
				if (itemData)
				{
					item->InitializeItemData(itemData);
				}
			}
		}
	}
}

// Called when the game starts or when spawned
void AMSpawnItemManager::BeginPlay()
{
	Super::BeginPlay();
}

FItemData* AMSpawnItemManager::GetDataFromDataTable(FItemTypeInfo ItemType)
{
	FItemData* itemData = nullptr;
	if (DataTable)
	{
		if (DataTable)
		{
			FString contextString;
			TArray<FName> RowNames;
			RowNames = DataTable->GetRowNames();

			for (auto& name : RowNames)
			{
				FItemData* row = DataTable->FindRow<FItemData>(name, contextString);
				if (row)
				{
					if (row->TypeInfo.Type == ItemType.Type && row->TypeInfo.Index == ItemType.Index)
					{
						return row;
					}
				}
			}
		}
	}
	return itemData;
}

