// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MSessionSubsystem.h"

void UMSessionSubsystem::Deinitialize()
{
	DestroySession();

	Super::Deinitialize();
}

void UMSessionSubsystem::SetSessionInfoFromGameMode()
{
	if (OpenLevelURL.IsEmpty())
	{
		if (UGameplayStatics::GetGameMode(GetWorld()))
		{
			if (AMGameMode* gameMode = Cast<AMGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				OpenLevelURL = gameMode->OpenLevelURL;
				CountOfMaxNumPlayers = gameMode->CountOfMaxNumPlayers;
			}
		}
	}
}

TArray<FOnlineSessionSearchResult> UMSessionSubsystem::GetFindSessions()
{
	return ArrayOfFindSessions;
}

void UMSessionSubsystem::StartSession()
{
	if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
		{
			sessionsPtrRef->OnStartSessionCompleteDelegates.AddUObject(this, &UMSessionSubsystem::OnStartSessionComplete);
			sessionsPtrRef->StartSession(GameSessionName);
		}
	}
}

void UMSessionSubsystem::EndSession()
{
	if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
		{
			sessionsPtrRef->OnEndSessionCompleteDelegates.AddUObject(this, &UMSessionSubsystem::OnEndSessionCompleted);
			sessionsPtrRef->EndSession(GameSessionName);
		}
	}
}

bool UMSessionSubsystem::IsCreateOrStartSession()
{
	return bCreateSession || bJoinToSession;
}

void UMSessionSubsystem::CreateSession(FName SessionName, bool bIsLAN, bool bIsDedicatedServer)
{
	if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
		{
			SetSessionInfoFromGameMode();

			FOnlineSessionSettings sessionSettings;
			sessionSettings.bIsDedicated = bIsDedicatedServer;
			sessionSettings.bAllowInvites = true;
			sessionSettings.bIsLANMatch = bIsLAN;
			sessionSettings.NumPublicConnections = CountOfMaxNumPlayers > 0 ? CountOfMaxNumPlayers : 2;
			sessionSettings.bUseLobbiesIfAvailable = false;
			sessionSettings.bUsesPresence = false;
			sessionSettings.bShouldAdvertise = true;
			sessionSettings.Set(FName("SESSION_NAME_KEY"), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

			sessionsPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UMSessionSubsystem::OnCreateSessionComplete);

			const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();

			sessionsPtrRef->CreateSession(*localPlayer->GetPreferredUniqueNetId(), SessionName, sessionSettings);
		}
	}
}

void UMSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
			{
				GameSessionName = SessionName;
				bCreateSession = true;
				sessionsPtrRef->OnCreateSessionCompleteDelegates.Clear();
				GetWorld()->ServerTravel(OpenLevelURL);
			}
		}
	}

}

void UMSessionSubsystem::DestroySession()
{
	if (!GameSessionName.ToString().IsEmpty())
	{
		if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
			{
				sessionsPtrRef->OnDestroySessionCompleteDelegates.AddUObject(this, &UMSessionSubsystem::OnDestroySessionComplete);
				if (!GameSessionName.ToString().IsEmpty() && GameSessionName != "None")
				{
					sessionsPtrRef->DestroySession(GameSessionName);
				}
			}
		}
	}
}

void UMSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
			{
				sessionsPtrRef->OnDestroySessionCompleteDelegates.Clear();
				GameSessionName = "";
				bJoinToSession = false;
				bCreateSession = false;
				UGameplayStatics::OpenLevel(GetWorld(), "MainMenuMap", true);
			}
		}
	}
}

void UMSessionSubsystem::FindSessions()
{
	if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
		{
			SessionSearch.Reset();
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, false, EOnlineComparisonOp::Equals);
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->bIsLanQuery = true;

			sessionsPtrRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UMSessionSubsystem::OnFindSessionsComplete);

			const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();

			sessionsPtrRef->FindSessions(*localPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
		}
	}
}

void UMSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	ArrayOfFindSessions.Empty();

	if (bWasSuccessful)
	{
		if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
			{
				sessionsPtrRef->OnFindSessionsCompleteDelegates.Clear();
				for (FOnlineSessionSearchResult Result : SessionSearch->SearchResults)
				{
					if (Result.IsValid())
					{
						ArrayOfFindSessions.Add(Result);
					}
				}
				
				ResultOfFindSessionsDelegate.Broadcast(SessionSearch->SearchResults.Num() > 0);
			}
		}
	}
}

void UMSessionSubsystem::JoinToSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
		{
			sessionsPtrRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UMSessionSubsystem::OnJoinSessionComplete);

			const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			sessionsPtrRef->JoinSession(*localPlayer->GetPreferredUniqueNetId(), SessionName, SearchResult);
		}
	}
}

void UMSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (APlayerController* playerController = GetWorld()->GetFirstPlayerController())
		{
			if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
			{
				if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
				{
					FString joinAdress;
					sessionsPtrRef->GetResolvedConnectString(SessionName, joinAdress);

					if (!joinAdress.IsEmpty())
					{
						GameSessionName = SessionName;
						bJoinToSession = true;
						sessionsPtrRef->OnJoinSessionCompleteDelegates.Clear();
						playerController->ClientTravel(joinAdress, ETravelType::TRAVEL_Absolute);
					}
				}
			}
		}
	}
}

void UMSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
			{
				sessionsPtrRef->OnStartSessionCompleteDelegates.Clear();
			}
		}
	}
}

void UMSessionSubsystem::OnEndSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (IOnlineSubsystem* onlineSub = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr sessionsPtrRef = onlineSub->GetSessionInterface())
			{
				sessionsPtrRef->OnEndSessionCompleteDelegates.Clear();
				if (!GameSessionName.ToString().IsEmpty())
				{
					DestroySession();
				}
			}
		}
	}
}
