// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/MPlayerState.h"

AMPlayerState::AMPlayerState()
{
	NetUpdateFrequency = 30.0f;
}

void AMPlayerState::SaveNewPlayerName(FString NewPlayerName)
{
	if (NewPlayerName != "")
	{
		PlayerInfo.SetPlayerName(NewPlayerName);
		SetPlayerName(NewPlayerName);
	}
}

void AMPlayerState::UpdatePlayerState()
{
	UMGameInstance* gameInstance = Cast<UMGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		PlayerInfo = gameInstance->GetPlayerInfoFromGameInstance();
		if (!PlayerInfo.PlayerName.IsEmpty())
		{
			SetPlayerName(PlayerInfo.PlayerName);
		}
	}
}

void AMPlayerState::SetCharacterInfo(ETypeOfCharacter Type)
{
	PlayerInfo.SetCharacterType(Type);
}

void AMPlayerState::SaveDataInGameInstance()
{
	UMGameInstance* gameInstance = Cast<UMGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		gameInstance->SavePlayerInfo(PlayerInfo);
	}
}

ETypeOfCharacter AMPlayerState::GetCharacterType()
{
	UpdatePlayerState();

	return PlayerInfo.CharacterType;
}

bool AMPlayerState::IsPlayerInSaveZone()
{
	return bPlayerInSaveZone;
}

void AMPlayerState::SetPlayerInSaveZone(bool bValue)
{
	bPlayerInSaveZone = bValue;
}

FPlayerInfoStruct& AMPlayerState::GetPlayerInfo()
{
	UpdatePlayerState();
	return PlayerInfo;
}

void AMPlayerState::SaveResultOfGame_Implementation(EResultOfGame ResultOfGame)
{
	PlayerInfo.ResultOfGame = ResultOfGame;
	SetCharacterInfo(ETypeOfCharacter::None);

	SaveDataInGameInstance();

	if (ResultOfGame != EResultOfGame::None)
	{
		UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>();

		if (sessionManager)
		{
			sessionManager->DestroySession();
		}
	}
}
