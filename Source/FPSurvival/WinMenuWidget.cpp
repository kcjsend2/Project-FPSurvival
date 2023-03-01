// Fill out your copyright notice in the Description page of Project Settings.


#include "WinMenuWidget.h"
#include "LevelLoadingGameInstanceSubsystem.h"

void UWinMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	TitleButton->OnClicked.AddDynamic(this, &UWinMenuWidget::OnTitleButtonClicked);
}

void UWinMenuWidget::OnTitleButtonClicked()
{
	const auto SubSystem = GetGameInstance()->GetSubsystem<ULevelLoadingGameInstanceSubsystem>();
	SubSystem->RequestLevelLoad(TEXT("Title"));
}

void UWinMenuWidget::SetKillCount(int KillCountParam)
{
	KillCount = KillCountParam;
}

void UWinMenuWidget::SetDamageDealt(int DamageDealtParam)
{
	DamageDealt = DamageDealtParam;
}

void UWinMenuWidget::SetDamageTaken(int DamageTakenParam)
{
	DamageTaken = DamageTakenParam;
}

void UWinMenuWidget::SetHidden()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UWinMenuWidget::SetVisible()
{
	SetVisibility(ESlateVisibility::Visible);
}
