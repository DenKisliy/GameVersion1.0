// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/MGameState.h"

void AMGameState::StartBeginPlayTimer()
{
	TypeOfTimer = ETypeOfTimer::StartMatch;
	TimerInterval = 1.0f;
	TimerCounter = GetTimeForTimerByType() + 1;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMGameState::OnTimerCounter, TimerInterval, true);
}

void AMGameState::StartCaptureStationTimer()
{
	TypeOfTimer = ETypeOfTimer::CaptureStation;
	TimerCounter = GetTimeForTimerByType() + 1;
}

void AMGameState::StartResurrectionTimer()
{
	TypeOfAdditionalTimer = ETypeOfAdditionalTimer::ResurrectionPlayer;
	AdditionalTimerCounter = GetAdditionalTimeForTimerByType() + 1;

	GetWorld()->GetTimerManager().SetTimer(AdditionalTimerHandle, this, &AMGameState::OnAdditionalTimerCounter, AdditionalTimerInterval, true);
}

void AMGameState::StopResurrectionTimer()
{
	AdditionalTimerCounter = -1;
}

void AMGameState::OnTimerCounter()
{
	TimerCounter = TimerCounter - 1;

	TimeChecker = TimerCounter;

	SetTimeForPlayers();
}

void AMGameState::StartSession_Implementation()
{
	if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
	{
		sessionManager->StartSession();
	}
}

void AMGameState::DestroySession_Implementation()
{
	if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
	{
		sessionManager->DestroySession();
	}
}

void AMGameState::SaveResultOfGame_Implementation(EResultOfGame ResultOfGame)
{
	for (APlayerState* playerStateBase : PlayerArray)
	{
		if (AMPlayerState* playerState = Cast<AMPlayerState>(playerStateBase))
		{
			playerState->SaveResultOfGame(ResultOfGame);
		}
	}
}

void AMGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (PlayerArray.Num() == GetPlayerCountFromGameMode())
	{
		for (APlayerState* playerStateBase : PlayerArray)
		{
			if (AMPlayerState* playerState = Cast<AMPlayerState>(playerStateBase))
			{
				playerState->PlayerDeathDelegate.AddDynamic(this, &AMGameState::OnPlayerDeath);
			}
		}

		if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
		{
			if (sessionManager->IsCreateOrStartSession())
			{
				StartBeginPlayTimer();
			}
		}
	}
}

void AMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGameState, TimeChecker);
	DOREPLIFETIME(AMGameState, AdditionalTimerChecker);
}

int AMGameState::GetPlayerCountFromGameMode()
{
	if (UGameplayStatics::GetGameMode(GetWorld()))
	{
		if (AMGameMode* gameMode = Cast<AMGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			return gameMode->CountOfMaxNumPlayers;
		}
	}

	return 0;
}

int AMGameState::GetTimeForTimerByType()
{
	int time = 0;

	if (UGameplayStatics::GetGameMode(GetWorld()))
	{
		if (AMGameMode* gameMode = Cast<AMGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			time = *gameMode->MainTimerMap.Find(TypeOfTimer);
		}
	}

	return time;
}

int AMGameState::GetAdditionalTimeForTimerByType()
{
	int time = 0;

	if (UGameplayStatics::GetGameMode(GetWorld()))
	{
		if (AMGameMode* gameMode = Cast<AMGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			time = *gameMode->AdditionalTimerMap.Find(TypeOfAdditionalTimer);
		}
	}

	return time;
}

void AMGameState::OnRep_TimeChecker()
{
	if (TimerCounter != TimeChecker)
	{
		TimerCounter = TimeChecker;

		SetTimeForPlayers();
	}
}

void AMGameState::SetTimeForPlayers()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	if (TimerCounter != -1 && GetPlayerCountFromGameMode() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMGameState::OnTimerCounter, TimerInterval, true);
	}

	ShowTimeForHUD(true, TimerCounter);

	if (TimerCounter == -1 && TypeOfTimer != ETypeOfTimer::None)
	{
		switch (TypeOfTimer)
		{
		case ETypeOfTimer::StartMatch:
			StartSession();
			GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(false);
			GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameAndUI());
			TimerAccelerationFactorDelegate.AddDynamic(this, &AMGameState::OnTimerAccelerationFactor);
			break;
		}
		TimerFinishDelegate.Broadcast(TypeOfTimer);
		TypeOfTimer = ETypeOfTimer::None;
		TimerInterval = 0;
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}

}

void AMGameState::OnTimerAccelerationFactor(float NewTimerPeriod)
{
	if (NewTimerPeriod == 0)
	{
		TimerCounter = -5;

		TimeChecker = TimerCounter;

		TypeOfTimer = ETypeOfTimer::None;
		TimerInterval = 0;

		SetTimeForPlayers();
	}

	if (NewTimerPeriod != 0 && TimerInterval != NewTimerPeriod)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		
		if (TypeOfTimer == ETypeOfTimer::None)
		{
			StartCaptureStationTimer();
		}
		TimerInterval = NewTimerPeriod;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMGameState::OnTimerCounter, TimerInterval, true);
	}
}

void AMGameState::OnPlayerDeath(bool bDeathPlayer)
{
	if (bDeathPlayer)
	{
		CountOfDeathPlayers = CountOfDeathPlayers + 1;
		if (CountOfDeathPlayers == GetPlayerCountFromGameMode())
		{
			SaveResultOfGame(EResultOfGame::Lost);
		}
	}
	else
	{
		CountOfDeathPlayers = CountOfDeathPlayers - 1;
		
		if (CountOfDeathPlayers < 0)
		{
			CountOfDeathPlayers = 0;
		}
	}
}

void AMGameState::OnRep_AdditionalTimerChecker()
{
	if (AdditionalTimerCounter != AdditionalTimerChecker)
	{
		AdditionalTimerCounter = AdditionalTimerChecker;

		SetAdditionalTimeForPlayers();
	}
}

void AMGameState::SetAdditionalTimeForPlayers()
{
	GetWorld()->GetTimerManager().ClearTimer(AdditionalTimerHandle);

	if (AdditionalTimerCounter != -1 && GetPlayerCountFromGameMode() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(AdditionalTimerHandle, this, &AMGameState::OnAdditionalTimerCounter, AdditionalTimerInterval, true);
	}

	ShowTimeForHUD(false, AdditionalTimerCounter);

	if (AdditionalTimerCounter == -1 && TypeOfAdditionalTimer != ETypeOfAdditionalTimer::None)
	{
		AdditionalTimerDelegate.Broadcast(TypeOfAdditionalTimer);
		TypeOfAdditionalTimer = ETypeOfAdditionalTimer::None;
		GetWorld()->GetTimerManager().ClearTimer(AdditionalTimerHandle);
	}
}

void AMGameState::OnAdditionalTimerCounter()
{
	AdditionalTimerCounter = AdditionalTimerCounter - 1;

	AdditionalTimerChecker = AdditionalTimerCounter;

	SetAdditionalTimeForPlayers();
}

void AMGameState::ShowTimeForHUD(bool bMain, int Time)
{
	if (IsValid(GetWorld()->GetFirstPlayerController()))
	{
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();

		if (IsValid(playerController->GetHUD()))
		{
			AMPlayerHUD* playerHUD = Cast<AMPlayerHUD>(playerController->GetHUD());

			if (IsValid(playerHUD))
			{
				playerHUD->SetTimeTimerWidget(bMain, Time);
			}
		}
	}
}
