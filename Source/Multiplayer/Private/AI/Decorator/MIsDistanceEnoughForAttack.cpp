// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/MIsDistanceEnoughForAttack.h"

bool UMIsDistanceEnoughForAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bValue = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
	
	const UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!blackboardComp)
	{
		return false;
	}

	if (AMAIController* controller = Cast<AMAIController>(OwnerComp.GetAIOwner()))
	{
		AMAIBaseCharacter* ownerCharacter = Cast<AMAIBaseCharacter>(controller->GetPawn());
		AMBaseCharacter* detectPlayer = Cast<AMBaseCharacter>(blackboardComp->GetValueAsObject("DetectPlayer"));
		
		if (ownerCharacter && detectPlayer)
		{
			FVector ownerLocation = ownerCharacter->GetActorLocation();
			FVector detectPlayerLocation = detectPlayer->GetActorLocation();
			return FMath::Sqrt(FMath::Pow(ownerLocation.X - detectPlayerLocation.X, 2) + FMath::Pow(ownerLocation.Y - detectPlayerLocation.Y, 2)) <= ownerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() + detectPlayer->GetCapsuleComponent()->GetScaledCapsuleRadius() + 10;
		}
	}

	return bValue;
}
