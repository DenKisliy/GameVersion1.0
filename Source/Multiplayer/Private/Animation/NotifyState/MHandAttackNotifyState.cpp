// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/NotifyState/MHandAttackNotifyState.h"

void UMHandAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	bHitByCharacter = false;
}

void UMHandAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!bHitByCharacter)
	{
		TArray<AActor*> actorsToIgnore;
		actorsToIgnore.Add(MeshComp->GetOwner());

		FHitResult hitLeftResult;
		FHitResult hitRightResult;

		bool bHitRight = UKismetSystemLibrary::SphereTraceSingle(MeshComp, MeshComp->GetSocketLocation("ik_hand_r_center"), MeshComp->GetSocketLocation("ik_hand_r"),
			12.f, UEngineTypes::ConvertToTraceType(ECC_Camera), false, actorsToIgnore, EDrawDebugTrace::None, hitRightResult, true, FLinearColor::Gray, FLinearColor::Blue, 1.f);
		
		bool bHitLeft = UKismetSystemLibrary::SphereTraceSingle(MeshComp, MeshComp->GetSocketLocation("ik_hand_l_center"), MeshComp->GetSocketLocation("ik_hand_l"),
			12.f, UEngineTypes::ConvertToTraceType(ECC_Camera), false, actorsToIgnore, EDrawDebugTrace::None, hitLeftResult, true, FLinearColor::Gray, FLinearColor::Blue, 1.f);

		if (bHitRight || bHitLeft)
		{
			AMBaseCharacter* playerWhichAttack = Cast<AMBaseCharacter>(MeshComp->GetOwner());

			if (playerWhichAttack)
			{
				if (playerWhichAttack)
				{
					bHitByCharacter = true;
					TSubclassOf<UMBaseAbility>* ability = playerWhichAttack->DefaultAbilities.FindByPredicate([](const TSubclassOf<UMBaseAbility>& InItem) {
						return InItem.GetDefaultObject()->AbilityInputID == EGDAbilityInputID::HandAttack;
						});

					if (ability)
					{
						ability->GetDefaultObject()->CalculateEffect(playerWhichAttack, bHitRight ? hitRightResult.GetActor() : hitLeftResult.GetActor());
					}
				}
			}
		}
	}
}