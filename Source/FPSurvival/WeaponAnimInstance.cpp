// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimInstance.h"
#include "WeaponBase.h"

void UWeaponAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningWeapon = Cast<AWeaponBase>(GetOwningActor());
}

void UWeaponAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	IsReloading = Montage_IsPlaying(OwningWeapon->WeaponReloadMontage)
					|| Montage_IsPlaying(OwningWeapon->WeaponEmptyReloadMontage);
	IsSlideStopValid = OwningWeapon->SlideStop != nullptr && OwningWeapon->WeaponRefPose != nullptr;
}
