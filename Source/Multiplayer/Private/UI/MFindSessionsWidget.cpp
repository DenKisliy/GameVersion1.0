// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MFindSessionsWidget.h"

void UMFindSessionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
	{
		sessionManager->ResultOfFindSessionsDelegate.AddDynamic(this, &UMFindSessionsWidget::CreateButtonForFindSession);
	}

	if (!IsValid(TextBlockOfInform))
	{
		CreateInformTextBlock();
	}
}

void UMFindSessionsWidget::CreateButtonForFindSession(bool bFindSession)
{
	VerticalBox->ClearChildren();
	TextBlockOfInform = nullptr;
	if (!IsValid(TextBlockOfInform))
	{
		CreateInformTextBlock();
		TextBlockOfInform->SetText(FText::FromString("Find sessions:"));
	}

	if (bFindSession)
	{
		if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
		{
			for (FOnlineSessionSearchResult Result : sessionManager->GetFindSessions())
			{
				UMJoinSessionWidget* button = CreateWidget<UMJoinSessionWidget>(this, JoinSessionWidget);
				button->CreateButton(Result);
				button->TypeDoesNotSetEvent.AddDynamic(this, &UMFindSessionsWidget::TypeDoesNotSet);
				VerticalBox->AddChild(button);
			}
		}
	}
	else
	{
		TextBlockOfInform->SetText(FText::FromString("Can not find any session"));
	}
}

void UMFindSessionsWidget::ShowInfoText(FString InfoText)
{
	VerticalBox->ClearChildren();
	TextBlockOfInform = nullptr;
	if (!IsValid(TextBlockOfInform))
	{
		CreateInformTextBlock();
	}

	TextBlockOfInform->SetText(FText::FromString(InfoText));
}

void UMFindSessionsWidget::TypeDoesNotSet()
{
	if (!IsValid(TextBlockOfInform))
	{
		CreateInformTextBlock();
	}

	TextBlockOfInform->SetText(FText::FromString("You don`t set character type!"));
}

void UMFindSessionsWidget::CreateInformTextBlock()
{
	TextBlockOfInform = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
	TextBlockOfInform->SetText(FText::FromString(""));

	UScaleBox* buttonTextScaleBox = NewObject<UScaleBox>(this, UScaleBox::StaticClass());
	buttonTextScaleBox->SetStretch(EStretch::ScaleToFit);

	USizeBox* buttonTextSizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
	buttonTextSizeBox->SetMinDesiredWidth(200.0f);
	buttonTextSizeBox->SetMinDesiredHeight(20.0f);
	buttonTextSizeBox->SetMaxDesiredHeight(40.0f);
	buttonTextSizeBox->AddChild(TextBlockOfInform);
	buttonTextScaleBox->AddChild(buttonTextSizeBox);
	VerticalBox->AddChild(buttonTextScaleBox);
}
