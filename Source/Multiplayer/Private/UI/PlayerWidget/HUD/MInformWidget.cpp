// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerWidget/HUD/MInformWidget.h"

void UMInformWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMInformWidget::SetText(FString Text)
{
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(Text));
	}
}
