// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZombieAIController.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZombieAIController();
	virtual void OnPossess(APawn* InPawn) override;

	static const FName PlayerCharacterKey;
	
private:
	UPROPERTY(EditAnywhere, Category = AI, meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BTAsset;

	UPROPERTY(EditAnywhere, Category = AI, meta = (AllowPrivateAccess = "true"))
	UBlackboardData* BBAsset;
};
