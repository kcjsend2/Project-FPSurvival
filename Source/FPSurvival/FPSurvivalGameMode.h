// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WaveManager.h"
#include "GameFramework/GameModeBase.h"
#include "FPSurvivalGameMode.generated.h"

class AZombieSpawner;

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	Ready,
	Progress,
	Pause
};


UCLASS(minimalapi)
class AFPSurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSurvivalGameMode();
	EWaveState GetWaveState() const { return WaveState; }
	
protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere)
	int MaxWave;
	
	UPROPERTY(EditAnywhere)
	TArray<int> EnemyCountForWave;
	
	int CurrentWave = 1;

	UPROPERTY(EditAnywhere)
	float WaveReadyDefaultTime;
	
	UPROPERTY(EditAnywhere)
	float WaveProgressDefaultTime;
	
	FTimespan WaveReadyRemainTime;
	FTimespan WaveProgressRemainTime;
	
	EWaveState WaveState = EWaveState::Ready;

	UPROPERTY()
	AZombieSpawner* ZombieSpawner;

	UPROPERTY(EditAnywhere)
	float ZombieSpawnInterval = 0.8;

	void WaveStart();
	void WaveEnd();

	UFUNCTION()
	void InitCharacter();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AZombieSpawner> ZombieSpawnerClass;
};