// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleScreenWidget.h"

#include "LevelLoadingGameInstanceSubsystem.h"

class ULevelLoadingGameInstanceSubsystem;

void UTitleScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StartButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnStartButtonClicked);
}

void UTitleScreenWidget::OnStartButtonClicked()
{
	const auto SubSystem = GetGameInstance()->GetSubsystem<ULevelLoadingGameInstanceSubsystem>();
	SubSystem->RequestLevelLoad(TEXT("InGameMap"));
}