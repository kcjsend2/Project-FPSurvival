// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelManager.h"

#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

void ULevelManager::LoadLevelAsync(const FString LevelName)
{
	auto& StreamableManager = UAssetManager::GetStreamableManager();
	const FSoftObjectPath LevelPath = LevelDir.ToString() + LevelName;
	StreamableHandle = StreamableManager.RequestAsyncLoad(LevelPath, FStreamableDelegate::CreateUFunction(this, "OnLevelLoadComplete", LevelName));
}

void ULevelManager::OnLevelLoadComplete(const FString LevelName) const
{
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

float ULevelManager::GetLevelLoadProgress() const
{
	if(StreamableHandle.IsValid())
	{
		return StreamableHandle->GetProgress();
	}
	return -1;
}
