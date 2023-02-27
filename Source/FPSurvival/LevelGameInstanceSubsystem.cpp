// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGameInstanceSubsystem.h"

ULevelGameInstanceSubsystem::ULevelGameInstanceSubsystem()
{
	LevelManager = CreateDefaultSubobject<ULevelManager>(TEXT("LevelManager"));
}

void ULevelGameInstanceSubsystem::RequestLevelLoad(FString LevelName) const
{
	LevelManager->LoadLevelAsync(LevelName);
}

float ULevelGameInstanceSubsystem::GetLevelLoadProgress() const
{
	return LevelManager->GetLevelLoadProgress();
}
