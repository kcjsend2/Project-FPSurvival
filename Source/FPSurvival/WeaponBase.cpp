// Copyright Epic Games, Inc. All Rights Reserved.


#include "WeaponBase.h"
#include "FPSurvivalCharacter.h"
#include "FPSurvivalProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "BulletProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "PickUpComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values for this component's properties
AWeaponBase::AWeaponBase()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	PickUpComponent = CreateDefaultSubobject<UPickUpComponent>(TEXT("PickUpComponent"));
	PickUpComponent->SetupAttachment(WeaponMesh);

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(WeaponMesh);
	
	static ConstructorHelpers::FClassFinder<AActor> SpawnedBulletClass(TEXT("/Game/FirstPerson/Blueprints/BP_BulletProjectile.BP_BulletProjectile_C"));
	BulletProjectileClass = SpawnedBulletClass.Class;
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

	PickUpComponent->OnPickUp.AddDynamic(this, &AWeaponBase::AttachWeapon);

	if(WeaponMesh->HasValidAnimationInstance())
		WeaponMesh->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AWeaponBase::MontageEnded);
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}


void AWeaponBase::Fire(AFPSurvivalCharacter* Character)
{
	if(!IsFiring && CurrentAmmo > 0)
	{
		IsFiring = true;
		IsFireAnimationEnd = false;
		
		if(ArmShootingMontage != nullptr)
		{
			Character->GetMesh1P()->GetAnimInstance()->Montage_Play(ArmShootingMontage);
		}
		
		if(WeaponShootingMontage != nullptr && WeaponMesh->HasValidAnimationInstance())
		{
			WeaponMesh->GetAnimInstance()->Montage_Play(WeaponShootingMontage);
		}
		UE_LOG(LogTemp, Log, TEXT("Fire"));

		FTransform MuzzleTransform;
		MuzzleTransform.SetLocation(Muzzle->GetComponentLocation());
		MuzzleTransform.SetRotation(Character->GetFirstPersonCameraComponent()->GetComponentTransform().GetRotation());
		MuzzleTransform.SetScale3D(FVector3d(1, 1, 1));
		
		const ABulletProjectile* SpawnedBullet = GetWorld()->SpawnActor<ABulletProjectile>(BulletProjectileClass, MuzzleTransform);

		CurrentAmmo--;
		UE_LOG(LogTemp, Log, TEXT("Current Ammo : %d"), CurrentAmmo);
		if(SpawnedBullet == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Spawn Failed"));
		}
	}
}

void AWeaponBase::ResolveReload(bool bInterrupted, AFPSurvivalCharacter* Character)
{
	if(!bInterrupted)
	{
		if(ReloadType == EReloadType::OneByOne)
		{
			CurrentAmmo++;
			if(MagazineLimit == CurrentAmmo && ArmReloadMontage != nullptr)
			{
				Character->GetMesh1P()->GetAnimInstance()->Montage_JumpToSection("ReturnPose", ArmReloadMontage);
				if(WeaponReloadMontage != nullptr && WeaponMesh->HasValidAnimationInstance())
					WeaponMesh->GetAnimInstance()->Montage_JumpToSection("ReturnPose", WeaponReloadMontage);
				Character->IsReloading = false;
				UE_LOG(LogTemp, Log, TEXT("ResolveReload: %s"), Character->IsReloading ? TEXT("true") : TEXT("false"));
			}
		}
		else if(ReloadType == EReloadType::WholeAtOnce)
		{
			Character->IsReloading = false;
			UE_LOG(LogTemp, Log, TEXT("ResolveReload: %s"), Character->IsReloading ? TEXT("true") : TEXT("false"));
			CurrentAmmo = MagazineLimit;
		}
		UE_LOG(LogTemp, Log, TEXT("Current Ammo : %d"), CurrentAmmo);
	}
}

void AWeaponBase::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == WeaponShootingMontage)
	{
		IsFiring = false;
	}
	else if(Montage == ArmShootingMontage && !bInterrupted)
	{
		IsFiring = false;
		IsFireAnimationEnd = true;
		
		FireOrReloadEnd.Execute();
	}
	else if(Montage == ArmReloadMontage && !bInterrupted)
	{
		FireOrReloadEnd.Execute();
	}
}

bool AWeaponBase::Reload(UAnimInstance* CharacterAnimInstance)
{
	if(ArmReloadMontage != nullptr && CurrentAmmo < MagazineLimit
		&& !CharacterAnimInstance->Montage_IsPlaying(nullptr) && IsFireAnimationEnd)
	{
		CharacterAnimInstance->Montage_Play(ArmReloadMontage);
		if(WeaponReloadMontage != nullptr && WeaponMesh->HasValidAnimationInstance())
			WeaponMesh->GetAnimInstance()->Montage_Play(WeaponReloadMontage);
		
		UE_LOG(LogTemp, Log, TEXT("Reload"));

		return true;
	}
	return false;
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AWeaponBase::AttachWeapon(AFPSurvivalCharacter* TargetCharacter)
{
	if(TargetCharacter != nullptr)
	{
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		AttachToComponent(TargetCharacter->GetMesh1P(), AttachmentRules, SocketName);

		const int WeaponSlot = TargetCharacter->CollectedWeapon.Num();
		
		TargetCharacter->OnFire[WeaponSlot].Clear();
		TargetCharacter->OnFire[WeaponSlot].BindDynamic(this, &AWeaponBase::Fire);
		
		TargetCharacter->OnReload[WeaponSlot].Clear();
		TargetCharacter->OnReload[WeaponSlot].BindDynamic(this, &AWeaponBase::Reload);
		
		TargetCharacter->CollectedWeapon.Add(this);

		TargetCharacter->OnWeaponChange(TargetCharacter->CollectedWeapon.Num() - 1);
		
		TargetCharacter->GetMesh1P()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AWeaponBase::MontageEnded);

		//TargetCharacter->CurrentWeaponSlot = TargetCharacter->CollectedWeapon.Num() - 1;

		FireOrReloadEnd.BindDynamic(TargetCharacter, &AFPSurvivalCharacter::SprintCheck);
		
		IsAttached = true;
	}
}