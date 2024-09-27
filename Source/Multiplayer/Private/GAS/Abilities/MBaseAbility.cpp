// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/MBaseAbility.h"

UAnimMontage* UMBaseAbility::GetRandomMontageAttack()
{
	TArray<int> probabilityArray;

	if (MontageAbilityArray.Num() > 1)
	{
		int randomValue = rand() % 100 + 1;
		for (int i = 0; i < MontageAbilityArray.Num(); i++)
		{
			probabilityArray.Add(probabilityArray.Num() > 0 ? (probabilityArray.Last() + MontageAbilityArray[i].Probability) : MontageAbilityArray[i].Probability);
		}

		for (int i = 0; i < probabilityArray.Num(); i++)
		{
			if (randomValue < probabilityArray[i] + 1)
			{
				return MontageAbilityArray[i].Montage;
			}
		}
	}
	else if (MontageAbilityArray.Num() == 1)
	{
		return MontageAbilityArray[0].Montage;
	}

	return nullptr;
}
