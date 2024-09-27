// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/MPlayerHUD.h"

AMPlayerHUD::AMPlayerHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AMPlayerHUD::BeginPlay()
{
	UMGameInstance* gameInstance = Cast<UMGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		FPlayerInfoStruct PlayerInfo = gameInstance->GetPlayerInfoFromGameInstance();
		bool bSetCharacterType = PlayerInfo.CharacterType != ETypeOfCharacter::None;
		
		if (IsValid(MainMenuStatic) && !bSetCharacterType)
		{
			GetOwningPlayerController()->SetShowMouseCursor(true);
			GetOwningPlayerController()->SetInputMode(FInputModeUIOnly());

			UMMainMenuWidget* mainMenuWidget = CreateWidget<UMMainMenuWidget>(this->GetOwningPlayerController(), MainMenuStatic);
			mainMenuWidget->AddToViewport();
		}

		if (bSetCharacterType)
		{
			if (IsValid(TimerStatic) && !IsValid(TimerWidget))
			{
				TimerWidget = CreateWidget<UMTimerWidget>(this->GetOwningPlayerController(), TimerStatic);
				TimerWidget->SetAnchorsInViewport(FAnchors(0.5f, 0.0f));
				TimerWidget->SetAlignmentInViewport(FVector2D(double(0.5), 0));
				TimerWidget->AddToViewport(2);
			}
			
			if (IsValid(AttributesGroupStatic))
			{
				AttributesGroupWidget = CreateWidget<UMAttributesGroupWidget>(this->GetOwningPlayerController(), AttributesGroupStatic);
				AttributesGroupWidget->AddToViewport();
			}
			
			if (IsValid(InformStatic))
			{
				InformWidget = CreateWidget<UMInformWidget>(this->GetOwningPlayerController(), InformStatic);
				InformWidget->AddToViewport();
			}

			GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(false);
			GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameAndUI());
		}

		if (PlayerInfo.ResultOfGame != EResultOfGame::None && !bSetCharacterType)
		{
			UMResultGameWidget* resultGameWidget = CreateWidget<UMResultGameWidget>(this->GetOwningPlayerController(), ResultGameStatic);
			resultGameWidget->SetResultOfGame(PlayerInfo.ResultOfGame == EResultOfGame::Win);
			resultGameWidget->AddToViewport(5);
		}
	}
}

void AMPlayerHUD::SetDelegateForSendMessageEvent(FScriptDelegate Delegate)
{
	UMGameInstance* gameInstance = Cast<UMGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		if (gameInstance->GetPlayerInfoFromGameInstance().CharacterType != ETypeOfCharacter::None)
		{
			if (!IsValid(ChatPanelWidget) && IsValid(ChatPanelStatic))
			{
				ChatPanelWidget = CreateWidget<UMChatPanelWidget>(this->GetOwningPlayerController(), ChatPanelStatic);
				if (!ChatPanelWidget->IsInViewport())
				{
					ChatPanelWidget->AddToViewport();
				}
			}

			if (IsValid(ChatPanelWidget))
			{
				ChatPanelWidget->SendMessageDelegate.Clear();
				ChatPanelWidget->SendMessageDelegate.Add(Delegate);
			}
		}
	}
}

void AMPlayerHUD::AddMessage(FString PlayerName, FString MessageText)
{
	if (IsValid(ChatPanelWidget))
	{
		ChatPanelWidget->AddMessage(PlayerName, MessageText);
	}
}

void AMPlayerHUD::SetTimeTimerWidget(bool bMain, int Time)
{
	if (!IsValid(TimerWidget))
	{
		TimerWidget = CreateWidget<UMTimerWidget>(this->GetOwningPlayerController(), TimerStatic);
		TimerWidget->SetAnchorsInViewport(FAnchors(0.5f, 0.0f));
		TimerWidget->SetAlignmentInViewport(FVector2D(double(0.5), 0));
		TimerWidget->AddToViewport(2);
	}
	TimerWidget->SetTimeForTimer(bMain, Time);
}

void AMPlayerHUD::SetValueForAttribute(EAttributeType Type, float Value)
{
	if (IsValid(AttributesGroupWidget))
	{
		AttributesGroupWidget->SetValueForAttribute(Type, Value);
	}
}

void AMPlayerHUD::SetRemoveOrAddAttributesGroupWidget(bool bAdd)
{
	if (IsValid(AttributesGroupWidget))
	{
		if (bAdd)
		{
			if (!AttributesGroupWidget->IsInViewport() && AttributesGroupWidget->GetParent())
			{
				AttributesGroupWidget->AddToViewport();
			}
			else
			{
				if (IsValid(AttributesGroupStatic) && !AttributesGroupWidget->IsInViewport())
				{
					AttributesGroupWidget = CreateWidget<UMAttributesGroupWidget>(this->GetOwningPlayerController(), AttributesGroupStatic);
					AttributesGroupWidget->AddToViewport();
				}
			}
		}
		else
		{
			if (AttributesGroupWidget->IsInViewport())
			{
				AttributesGroupWidget->RemoveFromParent();
			}
		}
	}
}

void AMPlayerHUD::ShowInventory(bool bValue)
{
	if (bValue)
	{
		if (IsValid(InventoryStatic))
		{
			if (!IsValid(InventoryWidget))
			{
				InventoryWidget = CreateWidget<UMInventoryWidget>(this->GetOwningPlayerController(), InventoryStatic);
				InventoryWidget->AddToViewport(-1);
			}
			else
			{
				InventoryWidget->RemoveFromParent();
				InventoryWidget = nullptr;
			}
		}
	}
	else
	{
		if (IsValid(InventoryWidget))
		{
			InventoryWidget->RemoveFromParent();
			InventoryWidget = nullptr;
		}
	}
}

void AMPlayerHUD::ShowInformText(FString Text)
{
	if (IsValid(InformWidget))
	{
		InformWidget->SetText(Text);
	}
}
