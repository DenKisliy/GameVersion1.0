// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MChatPanelWidget.h"

void UMChatPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	UVerticalBox* verticalBox = NewObject<UVerticalBox>(this, UVerticalBox::StaticClass());

	ScrollBox = NewObject<UScrollBox>(this, UScrollBox::StaticClass());
	ScrollBox->SetAlwaysShowScrollbar(true);
	ScrollBox->SetAllowOverscroll(true);
	verticalBox->AddChild(ScrollBox);
	
	TArray<UPanelSlot*> slotsArray = verticalBox->GetSlots();
	if (slotsArray.Num() > 0)
	{
		UVerticalBoxSlot* verticalSlot = Cast<UVerticalBoxSlot>(slotsArray[0]);
		
		FSlateChildSize slotSize;
		slotSize.SizeRule = ESlateSizeRule::Fill;
		slotSize.Value = 1;
		
		verticalSlot->SetSize(slotSize);
		verticalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		verticalSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
	}

	USizeBox* sizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
	sizeBox->SetHeightOverride(40);

	InputMessageBox = NewObject<UEditableTextBox>(this, UEditableTextBox::StaticClass());
	InputMessageBox->WidgetStyle = EditableTextStyle;
	InputMessageBox->SetHintText(FText::FromString("Enter text"));
	InputMessageBox->OnTextCommitted.AddDynamic(this, &UMChatPanelWidget::OnTextEntered);
	sizeBox->AddChild(InputMessageBox);

	verticalBox->AddChild(sizeBox);

	Border->AddChild(verticalBox);
}

void UMChatPanelWidget::AddMessage(FString PlayerName, FString MessageText)
{
	UMChatMessageWidget* chatMessageWidget = CreateWidget<UMChatMessageWidget>(this, ChatMessageWidget);
	chatMessageWidget->SetPlayerNameAndPlayerMessage(PlayerName, MessageText);

	ScrollBox->AddChild(chatMessageWidget);
	ScrollBox->ScrollToEnd();
}

void UMChatPanelWidget::OnTextEntered(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		SendMessageDelegate.Broadcast(GetOwningPlayerState()->GetPlayerName(), Text.ToString());

		InputMessageBox->SetText(FText::FromString(""));
		InputMessageBox->SetKeyboardFocus();
	}
}

UWidget* UMChatPanelWidget::GetInputMessageBox()
{
	return IsValid(InputMessageBox)? InputMessageBox : nullptr;
}
