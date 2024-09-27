// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Magic/MHealingAreaAbility.h"

UMHealingAreaAbility::UMHealingAreaAbility()
{
	AbilityAttackType = EGDAbilityAttackType::None;
	AbilityInputID = EGDAbilityInputID::HealingArea;
	AbilityType = EGDAbilityType::Healing;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.HealingArea")));
}

void UMHealingAreaAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (IsValid(SpawnActor))
		{
			AMHealingAreaActor* healingArea = GetWorld()->SpawnActor<AMHealingAreaActor>(SpawnActor, character->GetMesh()->GetComponentLocation(), character->GetActorRotation(), FActorSpawnParameters());
			healingArea->SetGameplayEffect(GetAbilityGameplayEffect().GetDefaultObject());

			float manaValue = character->Attributes->GetMana();

			character->Attributes->SetMana(manaValue - ManaCost);
		}
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}

}

bool UMHealingAreaAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
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
