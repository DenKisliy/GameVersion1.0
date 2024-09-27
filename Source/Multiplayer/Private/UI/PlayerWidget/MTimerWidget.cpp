// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerWidget/MTimerWidget.h"

void UMTimerWidget::SetTimeForTimer(bool bMain, int Time)
{
	FText textForTimer = GetTimeFortext(Time);
	if (bMain)
	{
		MainTimeText->SetVisibility(Time > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		MainTimeText->SetColorAndOpacity(Time > 5 ? FLinearColor::Black : FLinearColor::Red);
		MainTimeText->SetText(textForTimer);
	}
	else
	{
		AdditionalTimeText->SetVisibility(Time > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		AdditionalTimeText->SetColorAndOpacity(Time > 2 ? FLinearColor::Blue : FLinearColor::Red);
		AdditionalTimeText->SetText(textForTimer);

	}
}

FText UMTimerWidget::GetTimeFortext(int Time)
{
	FString timerText = "00:00";

	if (Time > 0)
	{
		int minutes = Time / 60;
		FString minutesText = minutes > 9 ? FString::FromInt(minutes) : "0" + FString::FromInt(minutes);

		int second = Time % 60;
		FString secondText = second > 9 ? FString::FromInt(second) : "0" + FString::FromInt(second);

		timerText = minutesText + ":" + secondText;
	}

	return FText::FromString(timerText);
}
