// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/NotifyState/MBiteNotifyState.h"

void UMBiteNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	bBiteCharacter = false;
}

void UMBiteNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!bBiteCharacter)
	{
		TArray<AActor*> actorsToIgnore;
		actorsToIgnore.Add(MeshComp->GetOwner());

		FHitResult hitResult;

		bool bHit = UKismetSystemLibrary::SphereTraceSingle(MeshComp, MeshComp->GetSocketLocation("Jaw"), MeshComp->GetSocketLocation("Jaw"),
			20.f, UEngineTypes::ConvertToTraceType(ECC_Camera), false, actorsToIgnore, EDrawDebugTrace::None, hitResult, true, FLinearColor::Gray, FLinearColor::Blue, 1.f);

		if (bHit)
		{
			AMBaseCharacter* characterWhichDamaged = Cast<AMBaseCharacter>(hitResult.GetActor());
			AMAIBaseCharacter* playerWhichAttack = Cast<AMAIBaseCharacter>(MeshComp->GetOwner());

			if (characterWhichDamaged && playerWhichAttack)
			{
				bBiteCharacter = true;
				for (FGameplayAbilitySpec ability : playerWhichAttack->GetAbilitySystemComponent()->GetActivatableAbilities())
				{
					if (UMBaseAbility* baseAbility = Cast<UMBaseAbility>(ability.Ability.Get()))
					{
						if (baseAbility->AbilityInputID == EGDAbilityInputID::Bite)
						{
							characterWhichDamaged->GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(baseAbility->GetAbilityGameplayEffect().GetDefaultObject(), 1.0f, playerWhichAttack->GetAbilitySystemComponent()->MakeEffectContext());
							break;
						}
					}
				}
			}
		}
	}
}
