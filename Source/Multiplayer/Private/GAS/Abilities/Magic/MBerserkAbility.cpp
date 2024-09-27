// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Magic/MBerserkAbility.h"

UMBerserkAbility::UMBerserkAbility()
{
	AbilityAttackType = EGDAbilityAttackType::None;
	AbilityType = EGDAbilityType::Buff;
	AbilityInputID = EGDAbilityInputID::Berserk;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Berserk")));
}

void UMBerserkAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		float mana = character->Attributes->GetMana();
		character->Attributes->SetMana(mana - ManaCost);
		float physicalBuff = character->Attributes->GetPhysicalBuff();
		character->Attributes->SetPhysicalBuff(physicalBuff + PercentageIncrease);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMBerserkAbility::TimerActiveAbility, TimerActiv, true);
	}
}

bool UMBerserkAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (character->Attributes)
		{
			return character->Attributes->GetMana() >= ManaCost;
		}
	}

	return false;
}

void UMBerserkAbility::TimerActiveAbility()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(GetCurrentActorInfo()->AvatarActor.Get()))
	{
		float physicalBuff = character->Attributes->GetPhysicalBuff();
		character->Attributes->SetPhysicalBuff(physicalBuff - PercentageIncrease);
	}
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}
