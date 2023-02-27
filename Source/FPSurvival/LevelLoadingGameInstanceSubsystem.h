// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelLoadingGameInstanceSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API ULevelLoadingGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	ULevelLoadingGameInstanceSubsystem();
	
protected:
	UPROPERTY()
	ULevelManager* LevelManager;
	
public:
	void RequestLevelLoad(FString LevelName) const;
	float GetLevelLoadProgress() const;
	
};
