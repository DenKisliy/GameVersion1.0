// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/NotifyState/MSwordAttackNotifyState.h"

void UMSwordAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	bHitByCharacter = false;
}

void UMSwordAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!bHitByCharacter)
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(MeshComp->GetOwner());

		FHitResult HitArray;

		bool bHit = UKismetSystemLibrary::SphereTraceSingle(MeshComp, MeshComp->GetSocketLocation("FX_Sword_Bottom"), MeshComp->GetSocketLocation("FX_Sword_Top"),
			12.f, UEngineTypes::ConvertToTraceType(ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::None, HitArray, true, FLinearColor::Gray, FLinearColor::Blue, 1.f);

		if (bHit)
		{
			AMBaseCharacter* playerWhichAttack = Cast<AMBaseCharacter>(MeshComp->GetOwner());

			if (playerWhichAttack)
			{
				bHitByCharacter = true;
				TSubclassOf<UMBaseAbility>* ability = playerWhichAttack->DefaultAbilities.FindByPredicate([](const TSubclassOf<UMBaseAbility>& InItem) {
					return InItem.GetDefaultObject()->AbilityInputID == EGDAbilityInputID::SwordAttack;
					});
				
				if (ability)
				{
					ability->GetDefaultObject()->CalculateEffect(playerWhichAttack, HitArray.GetActor());
				}
			}
		}
	}
}
