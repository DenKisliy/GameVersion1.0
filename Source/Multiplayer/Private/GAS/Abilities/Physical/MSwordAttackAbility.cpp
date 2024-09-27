// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Physical/MSwordAttackAbility.h"

UMSwordAttackAbility::UMSwordAttackAbility()
{
	AbilityType = EGDAbilityType::Attack;
	AbilityAttackType = EGDAbilityAttackType::Physical;
	AbilityInputID = EGDAbilityInputID::SwordAttack;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.SwordAttack")));
}

void UMSwordAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UAnimMontage* animation = GetRandomMontageAttack())
		{
			character->ServerAnimMontage(animation);
			float timeStempEnd = animation->GetPlayLength();
			FTimerHandle handle;
			GetWorld()->GetTimerManager().SetTimer(handle, [this, handle]()
				{
					CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
					GetWorld()->GetTimerManager().ClearTimer(const_cast<FTimerHandle&>(handle));
				}, timeStempEnd, false);
		}
	}
}

bool UMSwordAttackAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UMSwordAttackAbility::CalculateEffect(AActor* ActorWhichAttack, AActor* ActorWhichDamaged)
{
	AMBaseCharacter* characterAttack = Cast<AMBaseCharacter>(ActorWhichAttack);

	if (characterAttack)
	{
		FGameplayEffectContextHandle EffectContext = characterAttack->GetAbilitySystemComponent()->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = characterAttack->GetAbilitySystemComponent()->MakeOutgoingSpec(GetAbilityGameplayEffect(), 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
			if (Spec !=nullptr)
			{
				float physicalBuff = characterAttack->Attributes->GetPhysicalBuff();
				float magnitudeValue = physicalBuff > 0 ? (physicalBuff / 100 + 1) * GameplayEffectMagnitude : GameplayEffectMagnitude;
				Spec->SetSetByCallerMagnitude(GameplayEffectTag, magnitudeValue);
			}

			if (Cast<AMBaseCharacter>(ActorWhichDamaged))
			{
				AMBaseCharacter* characterDamaged = Cast<AMBaseCharacter>(ActorWhichDamaged);
				characterDamaged->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
			else if (Cast<AMAIBaseCharacter>(ActorWhichDamaged))
			{
				AMAIBaseCharacter* characterDamaged = Cast<AMAIBaseCharacter>(ActorWhichDamaged);
				characterDamaged->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
