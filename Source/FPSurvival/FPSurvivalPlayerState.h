// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSurvivalPlayerState.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class FPSURVIVAL_API AFPSurvivalPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void AddDamageDealt(float Damage);
	void AddDamageTaken(float Damage);
	void AddKillScore();

	float GetDamageDealt() const { return DamageDealt; }
	float GetDamageTaken() const { return DamageTaken; }
	int GetKillScore() const { return KillScore; }
	
protected:
	UPROPERTY()
	float DamageDealt = 0;

	UPROPERTY()
	float DamageTaken = 0;

	UPROPERTY()
	int KillScore = 0;
};
