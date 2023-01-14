// Fill out your copyright notice in the Description page of Project Settings.


#include "FPAnimInstance.h"
#include "FPSurvivalCharacter.h"
#include "WeaponBase.h"

void UFPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningPlayer = Cast<AFPSurvivalCharacter>(TryGetPawnOwner());
}

void UFPAnimInstance::AnimNotify_Reload()
{
    OwningPlayer->CurrentWeapon->ResolveReload(false);
}
