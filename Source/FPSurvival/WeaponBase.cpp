// Copyright Epic Games, Inc. All Rights Reserved.


#include "WeaponBase.h"
#include "FPSurvivalCharacter.h"
#include "FPSurvivalProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "PickUpComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values for this component's properties
AWeaponBase::AWeaponBase()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	PickUpComponent = CreateDefaultSubobject<UPickUpComponent>(TEXT("PickUpComponent"));
	PickUpComponent->SetupAttachment(WeaponMesh);
}

void AWeaponBase::BeginPlay()
{
	PickUpComponent->OnPickUp.AddDynamic(this, &AWeaponBase::AttachWeapon);
}

void AWeaponBase::Fire()
{
	IsFiring = true;
	
	if(Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	if(ArmShootingMontage != nullptr && WeaponShootingMontage != nullptr)
	{
		Character->GetMesh1P()->GetAnimInstance()->Montage_Play(ArmShootingMontage);
		WeaponMesh->GetAnimInstance()->Montage_Play(WeaponShootingMontage);
		
		UE_LOG(LogTemp, Log, TEXT("Fire"));
	}
}

void AWeaponBase::FireEnd()
{
	IsFiring = false;
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(Character != nullptr)
	{
		// Unregister from the OnUseItem Event
		Character->OnFire.RemoveDynamic(this, &AWeaponBase::Fire);
	}
}

void AWeaponBase::AttachWeapon(AFPSurvivalCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if(Character != nullptr)
	{
		// Attach the weapon to the First Person Character
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		AttachToComponent(Character->GetMesh1P(), AttachmentRules, SocketName);
		
		// Register so that Fire is called every time the character tries to use the item being held
		Character->OnFire.AddDynamic(this, &AWeaponBase::Fire);
		Character->OnFireEnd.AddDynamic(this, &AWeaponBase::FireEnd);
		Character->CollectedWeapon.Add(this);
		if(Character->CurrentWeapon != nullptr)
		{
			Character->CurrentWeapon->SetActorHiddenInGame(true); 
			Character->CurrentWeapon->SetActorEnableCollision(false); 
			Character->CurrentWeapon->SetActorTickEnabled(false);
		}
		Character->CurrentWeapon = this;
		//Character->GetMesh1P()->SetRelativeLocation(WeaponRelativePosition);
		//Character->GetMesh1P()->SetRelativeRotation(WeaponRelativeRotation);
		IsAttached = true;
	}
}