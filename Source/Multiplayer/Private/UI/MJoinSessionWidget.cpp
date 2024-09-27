// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MJoinSessionWidget.h"

void UMJoinSessionWidget::CreateButton(FOnlineSessionSearchResult SessionInfo)
{
	Session = SessionInfo;
	FString SessionName = "UMSessionButton";
	Session.Session.SessionSettings.Get(FName("SESSION_NAME_KEY"), SessionName);

	FString TextOfButton = (Session.Session.SessionSettings.bIsDedicated ? "Server:" : "Player:") + SessionName;
	SessionName.RemoveSpacesInline();

	UButton* button = NewObject<UButton>(this, UButton::StaticClass());

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

	button->OnClicked.AddDynamic(this, &UMJoinSessionWidget::OnClickButton);

	Border->AddChild(button);
}

void UMJoinSessionWidget::OnClickButton()
{
	if (AMPlayerState* playerState = Cast<AMPlayerState>(GetOwningPlayerState()))
	{
		playerState->SaveDataInGameInstance();
		if (playerState->GetCharacterType() != ETypeOfCharacter::None)
		{
			if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
			{
				FString SessionName = "";
				Session.Session.SessionSettings.Get(FName("SESSION_NAME_KEY"), SessionName);

				sessionManager->JoinToSession(FName(*SessionName), Session);
			}
		}
		else
		{
			TypeDoesNotSetEvent.Broadcast();
		}
	}
}
