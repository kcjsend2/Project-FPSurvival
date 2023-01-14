// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPAnimInstance.generated.h"

class AFPSurvivalCharacter;

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UFPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeInitializeAnimation() override;
	
public:
	UFUNCTION()
	void AnimNotify_Reload();

	UPROPERTY(BlueprintReadOnly, Category="Player")
	AFPSurvivalCharacter* OwningPlayer;
};
