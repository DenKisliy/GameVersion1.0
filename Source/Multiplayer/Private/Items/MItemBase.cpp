// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/MItemBase.h"

UMItemBase::UMItemBase()
{
	Count = 1;
}

void UMItemBase::GetIconTexture()
{
	if (IconPath.ToSoftObjectPath().IsValid())
	{
		FStreamableManager& streamableManager = UAssetManager::GetStreamableManager();
		streamableManager.RequestAsyncLoad(IconPath.ToSoftObjectPath(), [this]() {
			OnIconTextureLoadDelegate.ExecuteIfBound(
				IconPath.Get());
			});
	}
}

void UMItemBase::InitializeItemData(FItemData* NewItemData)
{
	Descriotion.SetData(NewItemData->Description.ItemName, NewItemData->Description.ItemDescription);
	TypeInfo = NewItemData->TypeInfo;
	IconPath = NewItemData->Asset.Icon;
	GameplayEffect = NewItemData->GameplayEffect;
}

void UMItemBase::ChangeCount(bool bAdd)
{
	if (bAdd)
	{
		Count = Count + 1;
	}
	else
	{
		Count = Count -1;
	}
}

bool UMItemBase::IsTypeMatches(FItemTypeInfo CheckType)
{
	return TypeInfo.Type == CheckType.Type && TypeInfo.Index == CheckType.Index;
}

int UMItemBase::GetCount()
{
	return Count;
}
