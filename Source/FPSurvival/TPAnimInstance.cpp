// Fill out your copyright notice in the Description page of Project Settings.


#include "TPAnimInstance.h"
#include "FPSurvivalCharacter.h"
#include "KismetAnimationLibrary.h"
#include "VaultingComponent.h"
#include "WeaponBase.h"

void UTPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningPlayer = Cast<AFPSurvivalCharacter>(TryGetPawnOwner());
}

void UTPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
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
			HasWeapon = true;
			IsFiring = CurrentWeapon->GetIsFiring();
			CurrentWeaponID = CurrentWeapon->WeaponID;
		}
		else
		{
			HasWeapon = false;
		}
		IsSprinting = OwningPlayer->IsSprinting();
		IsInSight = OwningPlayer->IsInSight;
		IsCrouching = OwningPlayer->IsCrouching();
		
		Speed = OwningPlayer->GetVelocity().Length();
		Direction = UKismetAnimationLibrary::CalculateDirection(OwningPlayer->GetVelocity(), OwningPlayer->GetActorRotation());

		if(Speed > 0)
			UE_LOG(LogTemp, Log, TEXT("Speed > 0"));

		CurrentState = OwningPlayer->GetCurrentMovementState();
	}
}
