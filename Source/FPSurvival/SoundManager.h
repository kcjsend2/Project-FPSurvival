// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "UObject/NoExportTypes.h"
#include "SoundManager.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API USoundManager : public UActorComponent
{
	GENERATED_BODY()

public:
	void AddSound(FName Key, USoundCue* Sound);
	void RemoveSound(FName Key);
	void PlaySound(FName Key, FVector Location, float Volume=1.f, float Pitch=1.f);
	
	UPROPERTY(EditAnywhere, Category="Sound")
	TMap<FName, USoundCue*> SoundMap;
};