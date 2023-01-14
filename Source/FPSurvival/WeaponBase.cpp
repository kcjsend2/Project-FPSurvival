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
}


void AWeaponBase::Fire()
{
	if(!IsFiring && CurrentAmmo > 0)
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

void AWeaponBase::ResolveReload(bool bInterrupted)
{
	if(!bInterrupted)
	{
		if(ReloadType == EReloadType::OneByOne)
		{
			CurrentAmmo++;
			if(MagazineLimit == CurrentAmmo && ArmReloadMontage != nullptr)
			{
				Character->GetMesh1P()->GetAnimInstance()->Montage_JumpToSection("ReturnPose", ArmReloadMontage);
				if(WeaponReloadMontage != nullptr)
					WeaponMesh->GetAnimInstance()->Montage_JumpToSection("ReturnPose", WeaponReloadMontage);
			}
		}
		else if(ReloadType == EReloadType::WholeAtOnce)
		{
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
		return;
	}
}

void AWeaponBase::Reload()
{
	if(ArmReloadMontage != nullptr && CurrentAmmo < MagazineLimit
		&& !Character->GetMesh1P()->GetAnimInstance()->Montage_IsPlaying(nullptr))
	{
	
		Character->GetMesh1P()->GetAnimInstance()->Montage_Play(ArmReloadMontage);
		if(WeaponReloadMontage != nullptr)
			WeaponMesh->GetAnimInstance()->Montage_Play(WeaponReloadMontage);
		
		UE_LOG(LogTemp, Log, TEXT("Reload"));
	}
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
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
		Character->OnFire.Clear();
		Character->OnFire.AddDynamic(this, &AWeaponBase::Fire);
		Character->CollectedWeapon.Add(this);
		if(Character->CurrentWeapon != nullptr)
		{
			Character->CurrentWeapon->SetActorHiddenInGame(true); 
			Character->CurrentWeapon->SetActorEnableCollision(false); 
			Character->CurrentWeapon->SetActorTickEnabled(false);
		}
		Character->CurrentWeapon = this;
		WeaponMesh->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AWeaponBase::MontageEnded);
		//Character->GetMesh1P()->SetRelativeLocation(WeaponRelativePosition);
		//Character->GetMesh1P()->SetRelativeRotation(WeaponRelativeRotation);
		IsAttached = true;
	}
}