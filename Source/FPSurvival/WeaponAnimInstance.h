// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponAnimInstance.generated.h"

class AWeaponBase;
/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UWeaponAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
	UPROPERTY(BlueprintReadOnly, Category="Player")
	AWeaponBase* OwningWeapon;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsReloading;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsSlideStopValid;
};
