// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerWidget/Item/MInventoryWidget.h"

void UMInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateItemsList();

	if (APlayerController* playerController = Cast<APlayerController>(GetOwningPlayer()))
	{
		if (AMBaseCharacter* player = Cast<AMBaseCharacter>(playerController->GetPawn()))
		{
			player->InventoryComponent->UpdateInventoryDelegate.AddDynamic(this, &UMInventoryWidget::CreateItemsList);
		}
	}
}

void UMInventoryWidget::CreateItemsList()
{
	ScrollBox->ClearChildren();
	TArray<UMItemBase*> itemsArray;
	if (APlayerController* playerController = Cast<APlayerController>(GetOwningPlayer()))
	{
		if (AMBaseCharacter* player = Cast<AMBaseCharacter>(playerController->GetPawn()))
		{
			itemsArray = player->InventoryComponent->GetItemsArray();
		}
	}

	if (itemsArray.Num() > 0)
	{
		int count = 0;
		UHorizontalBox* horizontalBox = nullptr;
		for (int i = 0; i< itemsArray.Num(); i++)
		{
			if (!horizontalBox)
			{
				horizontalBox = NewObject<UHorizontalBox>(this, UHorizontalBox::StaticClass());
			}
			
			UMItemWidget* itemWidget = CreateWidget<UMItemWidget>(GetOwningPlayer(), ItemStaticWidget);
			itemWidget->SetItemInfo(itemsArray[i]);
			horizontalBox->AddChild(itemWidget);
			count = count + 1;
			
			if (count == CountOfItemInOneRow)
			{
				ScrollBox->AddChild(horizontalBox);
				horizontalBox = nullptr;
				count = 0;
			}

			if (i == itemsArray.Num() - 1 && count < CountOfItemInOneRow && count> 0)
			{
				ScrollBox->AddChild(horizontalBox);
			}
		}
	}
}
