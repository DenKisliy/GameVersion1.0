// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MChatMessageWidget.h"

UMChatMessageWidget::UMChatMessageWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	PlayerName(""),
	PlayerMessage ("")
{
}

void UMChatMessageWidget::SetPlayerNameAndPlayerMessage(FString NewPlayerName, FString NewPlayerMessage)
{
	PlayerName = NewPlayerName;
	PlayerMessage = NewPlayerMessage;
}

void UMChatMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!PlayerName.IsEmpty() && !PlayerMessage.IsEmpty())
	{
		FString combineInfo = PlayerName + " : " + PlayerMessage;

		UTextBlock* textBlock = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
		textBlock->SetText(FText::FromString(combineInfo));

		HorizontalBox->AddChild(textBlock);
	}
}
