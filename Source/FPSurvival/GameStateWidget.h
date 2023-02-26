// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStateWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UGameStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WaveReadyString;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentWaveString;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WaveRemainTimeString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ZombieLeftString;

	int MaxWave;
	int CurrentWave;

	FTimespan WaveProgressRemainTime;
	FTimespan ReadyRemainTime;
};
