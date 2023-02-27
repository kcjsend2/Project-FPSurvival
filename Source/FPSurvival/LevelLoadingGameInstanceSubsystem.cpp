// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelLoadingGameInstanceSubsystem.h"

ULevelLoadingGameInstanceSubsystem::ULevelLoadingGameInstanceSubsystem()
{
	LevelManager = CreateDefaultSubobject<ULevelManager>(TEXT("LevelManager"));
}

void ULevelLoadingGameInstanceSubsystem::RequestLevelLoad(FString LevelName) const
{
	LevelManager->LoadLevelAsync(LevelName);
}

float ULevelLoadingGameInstanceSubsystem::GetLevelLoadProgress() const
{
	return LevelManager->GetLevelLoadProgress();
}
