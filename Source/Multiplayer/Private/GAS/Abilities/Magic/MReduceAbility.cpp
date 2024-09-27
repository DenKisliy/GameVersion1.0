// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Magic/MReduceAbility.h"

UMReduceAbility::UMReduceAbility()
{
	AbilityAttackType = EGDAbilityAttackType::None;
	AbilityType = EGDAbilityType::Buff;
	AbilityInputID = EGDAbilityInputID::Reduce;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reduce")));
}

void UMReduceAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsValid(Character))
	{
		Character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get());
		if (Character)
		{
			ActivateAbilityForCharacter();
			SetScaleForCharacter(true);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMReduceAbility::TimerActiveAbility, TimerInterval, true);
		}
	}
}

void UMReduceAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (ActorInfo != nullptr && ActorInfo->AvatarActor != nullptr)
	{
		SetScaleForCharacter(false);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UMReduceAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	Character = nullptr;
}

bool UMReduceAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return IsEnoughMana(ActorInfo);
}

void UMReduceAbility::TimerActiveAbility()
{
	if (!IsEnoughMana(CurrentActorInfo))
	{
		SetScaleForCharacter(false);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
	else
	{
		ActivateAbilityForCharacter();
	}
}

bool UMReduceAbility::IsEnoughMana(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (character->Attributes)
		{
			return character->Attributes->GetMana() >= ManaCostOfEachSecond;
		}
	}

	return false;
}

void UMReduceAbility::ActivateAbilityForCharacter()
{
	float manaValue = Character->Attributes->GetMana();

	Character->Attributes->SetMana(manaValue - ManaCostOfEachSecond);
}

void UMReduceAbility::SetScaleForCharacter(bool bScale)
{
	if (IsValid(Character))
	{
		float targetArmLength = Character->CameraBoom->TargetArmLength;
		Character->SetPlayerScale(bScale ? FVector(ReductionFactor) : FVector(1));
		Character->CameraBoom->TargetArmLength = bScale ? targetArmLength * ReductionFactor : targetArmLength / ReductionFactor;
	}
}
