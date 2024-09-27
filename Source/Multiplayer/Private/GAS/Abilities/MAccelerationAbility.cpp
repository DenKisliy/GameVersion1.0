// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/MAccelerationAbility.h"

UMAccelerationAbility::UMAccelerationAbility()
{
	AbilityAttackType = EGDAbilityAttackType::None;
	AbilityType = EGDAbilityType::Buff;
	AbilityInputID = EGDAbilityInputID::Acceleration;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Acceleration")));
}

void UMAccelerationAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsValid(Character))
	{
		Character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get());
		if (Character)
		{
			ActivateAbilityForCharacter();
			SetSpeedForCharacter(true);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMAccelerationAbility::TimerActiveAbility, TimerInterval, true);
		}
	}
}

void UMAccelerationAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (ActorInfo != nullptr && ActorInfo->AvatarActor != nullptr)
	{
		SetSpeedForCharacter(false);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UMAccelerationAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	Character = nullptr;
	DefaultSpeed = 0;
}

bool UMAccelerationAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return IsEnoughStamina(ActorInfo);
}

void UMAccelerationAbility::TimerActiveAbility()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("UMAccelerationAbility "), bIsActive ? TEXT("true") : TEXT("false"));
	if (!IsEnoughStamina(CurrentActorInfo))
	{
		SetSpeedForCharacter(false);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
	else
	{
		ActivateAbilityForCharacter();
	}
}

bool UMAccelerationAbility::IsEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (AMBaseCharacter* character = Cast<AMBaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (character->Attributes)
		{
			return character->Attributes->GetStamina() >= StaminaCostOfEachSecond;
		}
	}

	return false;
}

void UMAccelerationAbility::ActivateAbilityForCharacter()
{
	float staminaValue = Character->Attributes->GetStamina();

	Character->Attributes->SetStamina(staminaValue - StaminaCostOfEachSecond);
}

void UMAccelerationAbility::SetSpeedForCharacter(bool bAcceleration)
{
	if (DefaultSpeed == 0 && bAcceleration)
	{
		DefaultSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
	}

	if (AccelerationCoefficientSpeed > 0 && DefaultSpeed != 0)
	{
		Character->SetPlayerSpeed(bAcceleration ? DefaultSpeed * AccelerationCoefficientSpeed : DefaultSpeed);
	}
}
