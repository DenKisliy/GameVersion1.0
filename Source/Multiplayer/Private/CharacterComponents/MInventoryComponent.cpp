// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/MInventoryComponent.h"

// Sets default values for this component's properties
UMInventoryComponent::UMInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UMInventoryComponent::IsAddItemToInventory(FItemTypeInfo TypeInfo)
{
	bool bResult = false;
	if (ItemArray.Num() < 100)
	{
		if (!IsHaveItemInInventoryAdd(TypeInfo))
		{
			UMItemBase* newItem = NewObject<UMItemBase>(UMItemBase::StaticClass(), FName(*FString::FromInt(ItemArray.Num() + 1)));
			if (IsValid(newItem))
			{
				if (FItemData* data = GetDataFromDataTable(TypeInfo))
				{
					newItem->InitializeItemData(data);
					ItemArray.Add(newItem);
					bResult = true;
					UpdateInventoryDelegate.Broadcast();
				}
			}
		}
		else
		{
			bResult = true;
		}
	}
	
	return bResult;
}

void UMInventoryComponent::RemoveItemFromInventory(FItemTypeInfo TypeInfo)
{
	int findItemIndex = ItemArray.IndexOfByPredicate([this, TypeInfo](const UMItemBase* InItem)
		{
			return InItem->TypeInfo.Index == TypeInfo.Index && InItem->TypeInfo.Type == TypeInfo.Type;
		});

	if (ItemArray.IsValidIndex(findItemIndex))
	{
		if (ItemArray[findItemIndex]->GetCount() > 1)
		{
			ItemArray[findItemIndex]->ChangeCount(false);
		}
		else
		{
			ItemArray.RemoveAt(findItemIndex);
		}
		SpawnItemAfterRemove(TypeInfo);
	}
}

void UMInventoryComponent::UseItem(FItemTypeInfo TypeInfo)
{
	int findItemIndex = ItemArray.IndexOfByPredicate([this, TypeInfo](const UMItemBase* InItem)
		{
			return InItem->TypeInfo.Type == TypeInfo.Type && InItem->TypeInfo.Index == TypeInfo.Index;
		});

	if (ItemArray.IsValidIndex(findItemIndex))
	{
		if (IsValid(ItemArray[findItemIndex]->GameplayEffect))
		{
			UseItemDelegate.Broadcast(ItemArray[findItemIndex]->GameplayEffect);
		}

		if (ItemArray[findItemIndex]->GetCount() > 1)
		{
			ItemArray[findItemIndex]->ChangeCount(false);
		}
		else
		{
			ItemArray.RemoveAt(findItemIndex);
		}

		UpdateInventoryDelegate.Broadcast();
	}
}

FItemData* UMInventoryComponent::GetDataFromDataTable(FItemTypeInfo ItemType)
{
	FItemData* itemData = nullptr;
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
	return itemData;
}

TArray<UMItemBase*> UMInventoryComponent::GetItemsArray()
{
	return ItemArray;
}

void UMInventoryComponent::SetPickUpItem(FItemTypeInfo ItemType, const AActor* ItemActor)
{
	PickUpItemType = ItemType;
	PickUpItemActor = ItemType.Type == ETypeOfItem::None && ItemType.Index == 0 ? nullptr : const_cast<AActor*>(ItemActor);
}

void UMInventoryComponent::AddPickUpItemToInventory()
{
	if (PickUpItemType.Type != ETypeOfItem::None && PickUpItemType.Index != 0)
	{
		if (IsAddItemToInventory(PickUpItemType))
		{
			if (APawn* pawn = Cast<APawn>(GetOwner()))
			{
				if (AMPlayerState* playerState = Cast<AMPlayerState>(pawn->GetPlayerState()))
				{
					DestroyPickUpItem(PickUpItemActor);
					PickUpItemType = FItemTypeInfo();
				}
			}
		}
	}
}

void UMInventoryComponent::DestroyPickUpItem_Implementation(const AActor* ItemActor)
{
	if (AActor* item = const_cast<AActor*>(ItemActor))
	{
		item->Destroy();
	}
}

void UMInventoryComponent::SpawnItemAfterRemove_Implementation(const FItemTypeInfo& Type)
{
	if (AMGameMode* gameMode = Cast<AMGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		gameMode->SetSpawnItems(Type, GetOwner()->GetActorLocation());
	}
}

bool UMInventoryComponent::IsHaveItemInInventoryAdd(FItemTypeInfo ItemType)
{
	for (UMItemBase* newItem : ItemArray)
	{
		if (newItem->IsTypeMatches(ItemType))
		{
			newItem->ChangeCount(true);
			UpdateInventoryDelegate.Broadcast();
			return true;
		}
	}
	return false;
}

bool UMInventoryComponent::IsHaveItem(FItemTypeInfo ItemInfo)
{
	for (UMItemBase* newItem : ItemArray)
	{
		if (newItem->IsTypeMatches(ItemInfo))
		{
			return true;
		}
	}
	return false;
}
