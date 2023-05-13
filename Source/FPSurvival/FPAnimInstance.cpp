// Fill out your copyright notice in the Description page of Project Settings.


#include "FPAnimInstance.h"
#include "FPSurvivalCharacter.h"
#include "VaultingComponent.h"
#include "WeaponBase.h"

void UFPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningPlayer = Cast<AFPSurvivalCharacter>(TryGetPawnOwner());
}

void UFPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(OwningPlayer != nullptr)
	{
		IsInAir = OwningPlayer->GetMovementComponent()->IsFalling();
		IsMoving = OwningPlayer->GetVelocity().Size() > 0;
		IsVaulting = OwningPlayer->VaultingComponent->IsVaulting;

		const auto CurrentWeapon = OwningPlayer->CurrentWeapon;
		if(CurrentWeapon != nullptr)
		{
			IsFiring = CurrentWeapon->GetIsFiring();
			CurrentWeaponID = CurrentWeapon->WeaponID;
			AimDownSightOffset = CurrentWeapon->AimDownSightOffset;
		}
		IsSprinting = OwningPlayer->IsSprinting();
		IsInSight = OwningPlayer->IsInSight;

		VerticalVelocity = OwningPlayer->GetForwardAxis();
		HorizontalVelocity = OwningPlayer->GetRightAxis();
	}
}

void UFPAnimInstance::AnimNotify_Reload()
{
    OwningPlayer->CurrentWeapon->ResolveReload(false, OwningPlayer);
}

void UFPAnimInstance::AnimNotify_FireEnd()
{
	if(OwningPlayer->CurrentWeapon->FireMode == EFireMode::FullAuto)
	{
		OwningPlayer->FullAutoFireEnded();
	}
	else
	{
		if(OwningPlayer->OnFireEnd->ExecuteIfBound())
			OwningPlayer->ActionCheck();
	}
}

void UFPAnimInstance::AnimNotify_PullDownEnd()
{
	OwningPlayer->OnWeaponChangeEnd();
}
