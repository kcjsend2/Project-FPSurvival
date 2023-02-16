// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieCharacter.h"
#include "Animation/AnimInstance.h"
#include "ZombieAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UZombieAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadOnly, Category="Player")
	AZombieCharacter* OwningCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsMoving = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool IsSprinting = false;
};
