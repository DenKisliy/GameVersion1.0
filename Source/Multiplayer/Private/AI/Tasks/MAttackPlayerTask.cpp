// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/MAttackPlayerTask.h"

EBTNodeResult::Type UMAttackPlayerTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type nodeResult = EBTNodeResult::Failed;

	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (AMAIController* botController = Cast<AMAIController>(AIController))
		{
			if (botController->GetPawn())
			{
				if (AMAIBaseCharacter* ownerCharacter = Cast<AMAIBaseCharacter>(botController->GetPawn()))
				{
					if (ownerCharacter->DefaultAbilities.Num() > 0)
					{
						TArray<UMBaseAbility*> PhysicalAttackAbilities;
						for (TSubclassOf<UMBaseAbility>& Ability : ownerCharacter->DefaultAbilities)
						{
							if (Ability.GetDefaultObject()->AbilityAttackType == EGDAbilityAttackType::Physical && Ability.GetDefaultObject()->AbilityType == EGDAbilityType::Attack)
							{
								PhysicalAttackAbilities.Add(Ability.GetDefaultObject());
							}
						}

						UMBaseAbility* checkAbility = nullptr;
						if (PhysicalAttackAbilities.Num() == 1)
						{
							checkAbility = PhysicalAttackAbilities[0];
						}
						else if (PhysicalAttackAbilities.Num() > 1)
						{
							checkAbility = PhysicalAttackAbilities[FMath::RandRange(0, PhysicalAttackAbilities.Num() - 1)];
						}

						if (checkAbility)
						{
							ownerCharacter->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(checkAbility->AbilityTags) ? nodeResult = EBTNodeResult::Succeeded : nodeResult = EBTNodeResult::Failed;
						}
					}
				}
			}
		}
	}

	return nodeResult;
}
