// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPAnimInstance.generated.h"

class AFPSurvivalCharacter;
/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UTPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
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
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	float Speed;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	float Direction;
};
