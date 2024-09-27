// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerWidget/GAS/Attribute/MAttributesGroupWidget.h"

void UMAttributesGroupWidget::NativeConstruct()
{
	VerticalBox = NewObject<UVerticalBox>(this, UVerticalBox::StaticClass());

	HealthAttribute = CreateWidget<UMAttributeWidget>(this, AttributeWidget);
	HealthAttribute->SetColor(FLinearColor::Red);
	ManaAttribute = CreateWidget<UMAttributeWidget>(this, AttributeWidget);
	ManaAttribute->SetColor(FLinearColor::Blue);
	StaminaAttribute = CreateWidget<UMAttributeWidget>(this, AttributeWidget);
	StaminaAttribute->SetColor(FLinearColor::Green);

	VerticalBox->AddChild(HealthAttribute);
	VerticalBox->AddChild(ManaAttribute);
	VerticalBox->AddChild(StaminaAttribute);

	TArray<UPanelSlot*> slotsArray = VerticalBox->GetSlots();
	for (int i = 0; i < slotsArray.Num(); i++)
	{
		UVerticalBoxSlot* verticalSlot = Cast<UVerticalBoxSlot>(slotsArray[i]);

		verticalSlot->SetSize(FSlateChildSize());
		verticalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		verticalSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
	}

	CanvasPanel->AddChild(VerticalBox);
	slotsArray = CanvasPanel->GetSlots();
	for (int i = 0; i < slotsArray.Num(); i++)
	{
		UCanvasPanelSlot* canvasSlot = Cast<UCanvasPanelSlot>(slotsArray[i]);
		canvasSlot->SetAutoSize(true);
	}
}

void UMAttributesGroupWidget::SetValueForAttribute(EAttributeType Type, float Value)
{
	switch (Type)
	{
	case EAttributeType::Health:
	{
		if (IsValid(HealthAttribute))
		{
			HealthAttribute->SetPercent(Value);
		}
		break;
	}
	case EAttributeType::Mana:
	{
		if (IsValid(ManaAttribute))
		{
			ManaAttribute->SetPercent(Value);
		}
		break;
	}
	case EAttributeType::Stamina:
	{
		if (IsValid(StaminaAttribute))
		{
			StaminaAttribute->SetPercent(Value);
		}
		break;
	}
	}
}
