// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelGameInstanceSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API ULevelGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	ULevelGameInstanceSubsystem();

	void RequestLevelLoad(FString LevelName) const;
	float GetLevelLoadProgress() const;
	
protected:
	UPROPERTY()
	ULevelManager* LevelManager;

	bool bIsLoading = false;
};
