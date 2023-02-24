// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSurvivalGameMode.generated.h"

class AZombieSpawner;
UCLASS(minimalapi)
class AFPSurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSurvivalGameMode();
	
protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AZombieSpawner> ZombieSpawnerClass;
	
	UPROPERTY()
	AZombieSpawner* ZombieSpawner;
	
	UPROPERTY(EditAnywhere)
	int MaxWave;
	
	int CurrentWave;
};