// Fill out your copyright notice in the Description page of Project Settings.


#include "DeadMenuWidget.h"
#include "LevelLoadingGameInstanceSubsystem.h"

void UDeadMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//RestartButton->OnClicked.AddDynamic(this, &UDeadMenuWidget::OnRestartButtonClicked);
	TitleButton->OnClicked.AddDynamic(this, &UDeadMenuWidget::OnTitleButtonClicked);
}

// void UDeadMenuWidget::OnRestartButtonClicked()
// {
// 	OnGameRestart.Broadcast();
// }

void UDeadMenuWidget::OnTitleButtonClicked()
{
	const auto SubSystem = GetGameInstance()->GetSubsystem<ULevelLoadingGameInstanceSubsystem>();
	SubSystem->RequestLevelLoad(TEXT("Title"));
}

void UDeadMenuWidget::SetHidden()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UDeadMenuWidget::SetVisible()
{
	SetVisibility(ESlateVisibility::Visible);
}

