// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MPlayerInfoWidget.h"

void UMPlayerInfoWidget::SetPlayerName(FString PlayerName)
{
	NameTextBlock->SetText(FText::FromString(PlayerName));
}

FString UMPlayerInfoWidget::GetPlayerName()
{
	return NameTextBlock->GetText().ToString();
}
