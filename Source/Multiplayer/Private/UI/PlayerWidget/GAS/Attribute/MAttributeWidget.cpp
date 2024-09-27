// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerWidget/GAS/Attribute/MAttributeWidget.h"

void UMAttributeWidget::SetColor(FLinearColor Color)
{
	ProgressBar->SetPercent(1.0f);
	ProgressBar->SetFillColorAndOpacity(Color);

}

void UMAttributeWidget::SetPercent(float Value)
{
	ProgressBar->SetPercent(Value);
}
