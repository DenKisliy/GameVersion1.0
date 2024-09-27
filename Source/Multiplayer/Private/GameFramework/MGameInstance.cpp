// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/MGameInstance.h"

UMGameInstance::UMGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMGameInstance::SavePlayerInfo(FPlayerInfoStruct& NewPlayerInfo)
{
	PlayerInfo.UpdatePlayerInfo(NewPlayerInfo);
}

FPlayerInfoStruct& UMGameInstance::GetPlayerInfoFromGameInstance()
{
	return PlayerInfo;
}
