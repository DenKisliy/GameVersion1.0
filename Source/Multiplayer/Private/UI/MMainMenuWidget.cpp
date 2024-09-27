// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MMainMenuWidget.h"

void UMMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateButton("Login in", CreateDelegate(this, "LoginIn"));
}

void UMMainMenuWidget::CreateButton(FString NameOfButton, FScriptDelegate ClickDelegate)
{
	FString TextOfButton = NameOfButton;
	NameOfButton.RemoveSpacesInline();

	UButton* button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), FName(*NameOfButton));

	button->OnClicked.Add(ClickDelegate);

	UTextBlock* TextBlockOfButton = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
	TextBlockOfButton->SetText(FText::FromString(TextOfButton));

	UScaleBox* buttonTextScaleBox = NewObject<UScaleBox>(this, UScaleBox::StaticClass());
	buttonTextScaleBox->SetStretch(EStretch::ScaleToFill);

	USizeBox* buttonTextSizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
	buttonTextSizeBox->SetMinDesiredWidth(200.0f);
	buttonTextSizeBox->SetMinDesiredHeight(40.0f);
	buttonTextSizeBox->AddChild(TextBlockOfButton);
	buttonTextScaleBox->AddChild(buttonTextSizeBox);

	button->AddChild(buttonTextScaleBox);

	VerticalBox->AddChild(button);
}

void UMMainMenuWidget::CreateEditableTextBox(FString HintText, FScriptDelegate ChangeDelegate)
{
	UEditableText* PlayerNameTextBox = NewObject<UEditableText>(this, UEditableText::StaticClass());
	PlayerNameTextBox->SetHintText(FText::FromString(HintText));
	PlayerNameTextBox->OnTextChanged.Add(ChangeDelegate);

	VerticalBox->AddChild(PlayerNameTextBox);
}

void UMMainMenuWidget::CreateComboBox(FScriptDelegate SelectDelegate)
{
	ComboBoxWidget = NewObject<UComboBoxString>(this, UComboBoxString::StaticClass());

	uint8 countOfEnum = static_cast<uint8>(ETypeOfCharacter::Max);

	for (int i = 1; i < countOfEnum; i++)
	{
		FString value = UEnum::GetValueAsString(ETypeOfCharacter(i));
		FString type = typeid(ETypeOfCharacter).name();
		type = type + "::";
		type.RemoveFromStart("enum ");
		value.RemoveFromStart(type);
		ComboBoxWidget->AddOption(value);
	}
	ComboBoxWidget->SetItemStyle(ComboBoxItemStyle);
	ComboBoxWidget->OnSelectionChanged.Add(SelectDelegate);

	USizeBox* comboBoxSizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
	comboBoxSizeBox->SetMinDesiredWidth(200.0f);
	comboBoxSizeBox->SetMinDesiredHeight(40.0f);
	comboBoxSizeBox->AddChild(ComboBoxWidget);

	UScaleBox* comboBoxScaleBox = NewObject<UScaleBox>(this, UScaleBox::StaticClass());
	comboBoxScaleBox->SetStretch(EStretch::Fill);
	comboBoxScaleBox->AddChild(comboBoxSizeBox);

	VerticalBox->AddChild(comboBoxScaleBox);
}

FScriptDelegate UMMainMenuWidget::CreateDelegate(UObject* Object, FString FunctionName)
{
	if (!(FunctionName.IsEmpty()) && Object)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, FName(*FunctionName));
		return Delegate;
	}
	else
	{
		return FScriptDelegate();
	}
}

void UMMainMenuWidget::ValidateSessionNameText(const FText& Text)
{
	if (!Text.IsEmpty())
	{
		SessionName = Text.ToString();
	}
}

void UMMainMenuWidget::ValidatePlayerNameText(const FText& Text)
{
	if (!Text.IsEmpty())
	{
		if (AMPlayerState* playerState = Cast<AMPlayerState>(GetOwningPlayerState()))
		{
			playerState->SaveNewPlayerName(Text.ToString());
		}
	}
}

void UMMainMenuWidget::CreateSession()
{
	if (AMPlayerState* playerState = Cast<AMPlayerState>(GetOwningPlayerState()))
	{
		playerState->SaveDataInGameInstance();
		if (playerState->GetCharacterType() != ETypeOfCharacter::None)
		{

			if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
			{
				FString sessionName = SessionName.IsEmpty() ? GetOwningPlayerState()->GetPlayerName() : SessionName;
				sessionManager->CreateSession(FName(*sessionName), true, false);
			}
		}
		else
		{
			if (FindSessionResultWidget)
			{
				FindSessionResultWidget->ShowInfoText("You don`t set character type!");
			}
		}
	}
}

void UMMainMenuWidget::SelectCharacter(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	int32 selectItem = ComboBoxWidget->GetSelectedIndex() + 1;

	if (AMPlayerState* playerState = Cast<AMPlayerState>(GetOwningPlayerState()))
	{
		playerState->SetCharacterInfo(ETypeOfCharacter(selectItem));
	}
}

void UMMainMenuWidget::FindSessions()
{
	if (UMSessionSubsystem* sessionManager = GetGameInstance()->GetSubsystem<UMSessionSubsystem>())
	{
		if (FindSessionResultWidget)
		{
			FindSessionResultWidget->ShowInfoText("Search sessions");
		}
		sessionManager->FindSessions();
	}
}

void UMMainMenuWidget::LoginIn()
{
	VerticalBox->ClearChildren();
	
	CreateComboBox(CreateDelegate(this, "SelectCharacter"));
	CreateButton("Create session", CreateDelegate(this, "CreateSession"));
	CreateEditableTextBox("Session Name", CreateDelegate(this, "ValidateSessionNameText"));
	CreateEditableTextBox("Player Name", CreateDelegate(this, "ValidatePlayerNameText"));
	CreateButton("Find session", CreateDelegate(this, "FindSessions"));

	FindSessionResultWidget = CreateWidget<UMFindSessionsWidget>(this, FindSessionsWidget);
	VerticalBox->AddChild(FindSessionResultWidget);
}
