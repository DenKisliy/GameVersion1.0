// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerWidget/MResultGameWidget.h"

void UMResultGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMResultGameWidget::SetResultOfGame(bool bWin)
{
	if (!IsValid(ResultGameTextBlock))
	{
		UVerticalBox* verticalBox = NewObject<UVerticalBox>(this, UVerticalBox::StaticClass());

		ResultGameTextBlock = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
		ResultGameTextBlock->SetText(FText::FromString(""));

		UScaleBox* buttonTextScaleBox = NewObject<UScaleBox>(this, UScaleBox::StaticClass());
		buttonTextScaleBox->SetStretch(EStretch::ScaleToFill);

		USizeBox* buttonTextSizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
		buttonTextSizeBox->SetMinDesiredWidth(200.0f);
		buttonTextSizeBox->SetMinDesiredHeight(40.0f);
		buttonTextSizeBox->AddChild(ResultGameTextBlock);
		buttonTextScaleBox->AddChild(buttonTextSizeBox);

		verticalBox->AddChild(buttonTextScaleBox);

		FScriptDelegate delegate;
		delegate.BindUFunction(this, FName("CloseWindow"));
		verticalBox->AddChild(CreateButton("Ok", delegate));

		Border->AddChild(verticalBox);
	}

	ResultGameTextBlock->SetColorAndOpacity(bWin ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red));
	ResultGameTextBlock->SetText(FText::FromString(bWin ? "You won!" : "You lost!"));
}

void UMResultGameWidget::CloseWindow()
{
	AMPlayerState* playerState = Cast<AMPlayerState>(GetOwningPlayerState());
	if (playerState)
	{
		playerState->SetCharacterInfo(ETypeOfCharacter::None);
		playerState->SaveResultOfGame(EResultOfGame::None);
	}

	RemoveFromParent();
}

UButton* UMResultGameWidget::CreateButton(FString NameOfButton, FScriptDelegate ClickDelegate)
{
	FString TextOfButton = NameOfButton;
	NameOfButton.RemoveSpacesInline();

	UButton* button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), FName(*NameOfButton));

	button->OnClicked.Add(ClickDelegate);

	UTextBlock* TextBlockOfButton = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
	TextBlockOfButton->SetText(FText::FromString(TextOfButton));

	UScaleBox* buttonTextScaleBox = NewObject<UScaleBox>(this, UScaleBox::StaticClass());
	buttonTextScaleBox->SetStretch(EStretch::ScaleToFill);

	USizeBox* buttonTextSizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
	buttonTextSizeBox->SetMinDesiredWidth(200.0f);
	buttonTextSizeBox->SetMinDesiredHeight(40.0f);
	buttonTextSizeBox->AddChild(TextBlockOfButton);
	buttonTextScaleBox->AddChild(buttonTextSizeBox);

	button->AddChild(buttonTextScaleBox);

	return button;
}
