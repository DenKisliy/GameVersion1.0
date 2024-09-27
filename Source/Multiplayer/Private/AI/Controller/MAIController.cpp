// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/MAIController.h"

AMAIController::AMAIController(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	UAISenseConfig_Sight* sight = CreateDefaultSubobject<UAISenseConfig_Sight>("Sensesight");
	sight->SightRadius = 100;
	sight->LoseSightRadius = 150;
	sight->PeripheralVisionAngleDegrees = 90;
	sight->DetectionByAffiliation.bDetectNeutrals = true;
	sight->DetectionByAffiliation.bDetectFriendlies = true;
	sight->DetectionByAffiliation.bDetectEnemies = true;

	UAISenseConfig_Hearing* hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>("SenseHearing");
	hearing->HearingRange = 100;
	hearing->DetectionByAffiliation.bDetectNeutrals = true;
	hearing->DetectionByAffiliation.bDetectFriendlies = true;
	hearing->DetectionByAffiliation.bDetectEnemies = true;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComponent");
	AIPerceptionComponent->ConfigureSense(*sight);
	AIPerceptionComponent->ConfigureSense(*hearing);
	AIPerceptionComponent->SetDominantSense(sight->GetSenseImplementation());

	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMAIController::OnPerception);

	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AMAIController::SetStartDate()
{
	if (AMAIBaseCharacter* bot = Cast<AMAIBaseCharacter>(GetPawn()))
	{
		OwnerCharacter = bot;
		OwnerCharacter->StartMoveDelegate.AddDynamic(this, &AMAIController::LaunchBehaviorTree);
	}
}

void AMAIController::LaunchBehaviorTree()
{
	if (BTPatrol)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, "LaunchBehaviorTree");
		OwnerCharacter->StartMoveDelegate.Remove(Delegate);
		RunBehaviorTree(BTPatrol);
	}
}

void AMAIController::OnPerception(AActor* Actor, FAIStimulus Stimulus)
{
	if (InitializeBlackboardComponent(BTStalkingAndAttack))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (AMBaseCharacter* player = Cast<AMBaseCharacter>(Actor))
			{
				if (AMPlayerState* playerState = Cast<AMPlayerState>(player->GetPlayerState()))
				{
					if (!playerState->IsPlayerInSaveZone())
					{
						BlackboardComponent->SetValueAsObject("DetectPlayer", Actor);
						if (!player->InsertInSaveZoneDelegate.IsBound())
						{
							player->InsertInSaveZoneDelegate.AddDynamic(this, &AMAIController::OnDetectPlayerInSaveZone);
						}
						OwnerCharacter->DetectPlayerDelegate.Broadcast(OwnerCharacter, player);
						RunBehaviorTree(BTStalkingAndAttack);
					}
				}
			}
		}
		else
		{
			BlackboardComponent->SetValueAsObject("DetectPlayer", nullptr);
			OwnerCharacter->DetectPlayerDelegate.Broadcast(OwnerCharacter, nullptr);
			RunBehaviorTree(BTPatrol);
		}
	}
}

bool AMAIController::InitializeBlackboardComponent(UBehaviorTree* BehaviorTree)
{
	if (UBlackboardData* blackboardData = BTStalkingAndAttack->GetBlackboardAsset())
	{
		BlackboardComponent->InitializeBlackboard(*blackboardData);
		return true;
	}

	return false;
}

void AMAIController::OnDetectPlayerInSaveZone(AActor* Player)
{
	if (InitializeBlackboardComponent(BTStalkingAndAttack))
	{
		if (AMBaseCharacter* player = Cast<AMBaseCharacter>(Player))
		{
			BlackboardComponent->SetValueAsObject("DetectPlayer", nullptr);
			RunBehaviorTree(BTPatrol);
		}
	}
}

void AMAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetStartDate();
}

void AMAIController::FindNextPoint(FVector& NextPoint, int& Index)
{
	if (IsValid(OwnerCharacter))
	{
		OwnerCharacter->FindNextPointAndIndex(NextPoint, Index);
	}
}
