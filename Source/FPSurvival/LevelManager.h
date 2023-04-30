// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "UObject/NoExportTypes.h"
#include "LevelManager.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API ULevelManager : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FSoftObjectPath LevelDir = FSoftObjectPath(TEXT("/Game/FirstPerson/Maps/"));

	TSharedPtr<FStreamableHandle, ESPMode::ThreadSafe> StreamableHandle;

	UFUNCTION()
	void OnLevelLoadComplete(const FString LevelName) const;
	
public:
	void LoadLevelAsync(const FString LevelName);
	float GetLevelLoadProgress() const;
};
