// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Physical/MBiteAbility.h"

UMBiteAbility::UMBiteAbility()
{
	AbilityType = EGDAbilityType::Attack;
	AbilityAttackType = EGDAbilityAttackType::Physical;
	AbilityInputID = EGDAbilityInputID::Bite;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Bite")));
}

void UMBiteAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AMAIBaseCharacter* character = Cast<AMAIBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UAnimMontage* animation = GetRandomMontageAttack())
		{
			character->MulticastPlayMontage(animation);
			float timeStempEnd = animation->GetPlayLength();

			FTimerHandle handle;
			if (handle.IsValid())
			{
				GetWorld()->GetTimerManager().SetTimer(handle, [this, handle]()
					{
						CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);

						GetWorld()->GetTimerManager().ClearTimer(const_cast<FTimerHandle&>(handle));
					}, timeStempEnd, false);
			}
		}
	}
}

bool UMBiteAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}