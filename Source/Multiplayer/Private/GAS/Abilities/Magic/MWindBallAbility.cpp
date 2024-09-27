// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Magic/MWindBallAbility.h"

UMWindBallAbility::UMWindBallAbility()
{
	AbilityAttackType = EGDAbilityAttackType::Magic;
	AbilityInputID = EGDAbilityInputID::WindBall;
	AbilityType = EGDAbilityType::Attack;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.WindBall")));
}

void UMWindBallAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		AMWindBallActor* windBall = GetWorld()->SpawnActor<AMWindBallActor>(SpawnActor, character->SpawnArrowComponent->GetComponentLocation(), character->GetActorRotation(), FActorSpawnParameters());
		windBall->SetDamageEffect(GetAbilityGameplayEffect().GetDefaultObject());

		float manaValue = character->Attributes->GetMana();

		character->Attributes->SetMana(manaValue - ManaCost);
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

bool UMWindBallAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return IsEnoughMana(ActorInfo);
}

bool UMWindBallAbility::IsEnoughMana(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (character->Attributes)
		{
			return character->Attributes->GetMana() >= ManaCost;
		}
	}

	return false;
}
