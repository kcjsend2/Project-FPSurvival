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
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
	UFUNCTION()
	void AnimNotify_Reload();
	
	UFUNCTION()
	void AnimNotify_WeaponChange();

	UPROPERTY(BlueprintReadOnly, Category="Player")
	AFPSurvivalCharacter* OwningPlayer;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsMoving = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsInAir = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsVaulting = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	int CurrentWeaponID = -1;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsFiring = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsFullAuto = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsSprinting = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsInSight = false;
};
