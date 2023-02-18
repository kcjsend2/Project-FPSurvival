// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSurvivalCharacter.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ZombieDetectPlayer.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UBTService_ZombieDetectPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ZombieDetectPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY()
	AFPSurvivalCharacter* Character;
	
	UPROPERTY(EditAnywhere)
	float DetectRadius;
};
