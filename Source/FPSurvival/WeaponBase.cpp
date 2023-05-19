// Copyright Epic Games, Inc. All Rights Reserved.


#include "WeaponBase.h"
#include "FPSurvivalCharacter.h"
#include "FPSurvivalProjectile.h"
#include "BulletProjectile.h"
#include "DataTableManageSubsystem.h"
#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "WeaponPickUpComponent.h"
#include "Camera/CameraComponent.h"
#include "SoundManager.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values for this component's properties
AWeaponBase::AWeaponBase()
{
	SoundManager = CreateDefaultSubobject<USoundManager>(TEXT("SoundManager"));

	DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	SetRootComponent(DefaultSceneComponent);
	
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(DefaultSceneComponent);
	
	FPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPWeaponMesh"));
	FPWeaponMesh->SetupAttachment(DefaultSceneComponent);
	FPWeaponMesh->bCastDynamicShadow = false;
	FPWeaponMesh->CastShadow = false;
	FPWeaponMesh->SetSimulatePhysics(true);
	
	MagazineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagazineMesh"));
	MagazineMesh->SetupAttachment(FPWeaponMesh, TEXT("SOCKET_Magazine"));

	DefaultScope = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DefaultScopeMesh"));
	DefaultScope->SetupAttachment(FPWeaponMesh, TEXT("SOCKET_Default"));
	
	TPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TPWeaponMesh"));
	TPWeaponMesh->SetupAttachment(DefaultSceneComponent);
	TPWeaponMesh->SetVisibility(false);
	TPWeaponMesh->bCastDynamicShadow = true;
	TPWeaponMesh->CastShadow = true;
	
	PickUpComponent = CreateDefaultSubobject<UWeaponPickUpComponent>(TEXT("PickUpComponent"));
	PickUpComponent->SetupAttachment(FPWeaponMesh);
	
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(FPWeaponMesh);

	MuzzleFlash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MuzzleFlashParticle"));
	MuzzleFlash->SetupAttachment(Muzzle);
	MuzzleFlash->SetAutoActivate(false);
	
	static ConstructorHelpers::FClassFinder<AActor> SpawnedBulletClass(TEXT("/Game/FirstPerson/Blueprints/BP_BulletProjectile.BP_BulletProjectile_C"));
	BulletProjectileClass = SpawnedBulletClass.Class;
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    const FWeaponData* WeaponData = GetGameInstance()->GetSubsystem<UDataTableManageSubsystem>()->GetWeaponInitData(WeaponID);
	SetWeaponData(WeaponData);
	
	PickUpComponent->OnPickUp.AddDynamic(this, &AWeaponBase::AttachWeapon);

	SoundManager->SetAudioComponent(AudioComponent);
	
	if(FPWeaponMesh->HasValidAnimationInstance())
		FPWeaponMesh->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AWeaponBase::MontageEnded);

}

void AWeaponBase::SetWeaponData(const FWeaponData* WeaponData)
{
	MagazineLimit = WeaponData->MagazineLimit;
	FPSocketName = WeaponData->FPSocketName;
	TPSocketName = WeaponData->TPSocketName;
	BulletDamage = WeaponData->BulletDamage;
	RecoilYaw = WeaponData->RecoilYaw;
	RecoilPitch = WeaponData->RecoilPitch;
	RecoilPitchADS = WeaponData->RecoilPitchADS;
	FireMode = WeaponData->FireMode;
	ReloadType = WeaponData->ReloadType;
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AWeaponBase::Fire(AFPSurvivalCharacter* Character)
{
	if(FireMode == EFireMode::Single && IsFiring)
		return;

	if(CurrentAmmo <= 0)
	{
		SoundManager->PlaySound(TEXT("Empty"), GetActorLocation());
		return;
	}
	
	IsFiring = true;

	Character->CrosshairWidget->FireSpreadValue += SpreadPerShot;
	
	if(ArmFireMontage != nullptr)
		Character->GetMesh1P()->GetAnimInstance()->Montage_Play(ArmFireMontage);
	
	if(WeaponFireMontage != nullptr && FPWeaponMesh->HasValidAnimationInstance())
		FPWeaponMesh->GetAnimInstance()->Montage_Play(WeaponFireMontage);
	
	UE_LOG(LogTemp, Log, TEXT("Fire"));

	FTransform MuzzleTransform;
	MuzzleTransform.SetLocation(Muzzle->GetComponentLocation());
	MuzzleTransform.SetRotation(Character->GetFirstPersonCameraComponent()->GetComponentTransform().GetRotation());
	MuzzleTransform.SetScale3D(FVector3d(1, 1, 1));
	
	ABulletProjectile* SpawnedBullet = GetWorld()->SpawnActor<ABulletProjectile>(BulletProjectileClass, MuzzleTransform);
	SpawnedBullet->BulletDamage = BulletDamage;
	SpawnedBullet->LocationFired = GetActorLocation();

	if(RecoilOn)
		Character->RecoilTimeline->PlayFromStart();

	MuzzleFlash->ActivateSystem(true);
	
	FHitResult HitResult;
	const FVector Start = Character->GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + Character->GetFirstPersonCameraComponent()->GetForwardVector() * BulletRange;

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Character);
	bool Hit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionQueryParams);
	if(Hit)
	{
		//DrawDebugLine(GetWorld(), HitResult.TraceStart, HitResult.TraceEnd, FColor(255, 0, 0), false, 2.f, 0.f, 1.f);
		AFPSurvivalCharacter* HitCharacter = Cast<AFPSurvivalCharacter>(HitResult.GetActor());
		AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(HitResult.GetActor());

		AActor* HitActor = nullptr;
		float CurrentHP = -999;
		if(HitCharacter != nullptr)
		{
			CurrentHP = HitCharacter->GetCurrentHP();
			HitActor = HitCharacter;
		}
		else if(HitEnemy != nullptr)
		{
			CurrentHP = HitEnemy->GetCurrentHP();
			HitActor = HitEnemy;
		}
		
		if(HitActor != nullptr)
		{
			FVector HitDirection = Character->GetActorLocation() - HitActor->GetActorLocation();
			HitDirection.Normalize();

			float ResultDamage = BulletDamage;
			if(HitResult.BoneName == "head")
			{
				ResultDamage *= HeadshotMultiplier;
			}
			else
			{
				for(int i = 0; i < LimbshotBones.Num(); ++i)
				{
					if(HitResult.BoneName == LimbshotBones[i])
					{
						ResultDamage *= LimbshotMultiplier;
						break;
					}
				}
			}
			UGameplayStatics::ApplyPointDamage(HitActor, ResultDamage, HitDirection, HitResult, GetInstigatorController(), this, nullptr);
			Character->DamageToOtherActor(HitResult.BoneName == "head" ? true : false, CurrentHP - ResultDamage <= 0 ? true : false, ResultDamage);
		}
	}

	SoundManager->PlaySound(TEXT("Fire"), GetActorLocation());
	CurrentAmmo--;

	if(CurrentAmmo == 0)
		SoundManager->PlaySound(TEXT("Empty"), GetActorLocation());
	
	UE_LOG(LogTemp, Log, TEXT("Current Ammo : %d"), CurrentAmmo);
	
	if(SpawnedBullet == nullptr)
		UE_LOG(LogTemp, Log, TEXT("Spawn Failed"));
}

void AWeaponBase::FireEnd()
{
	IsFiring = false;
}

void AWeaponBase::ResolveReload(bool bInterrupted, AFPSurvivalCharacter* Character)
{
	if(!bInterrupted)
	{
		if(ReloadType == EReloadType::OneByOne)
		{
			Character->AmmoMap[WeaponName]--;
			CurrentAmmo++;
			if(MagazineLimit == CurrentAmmo && ArmReloadMontage != nullptr)
			{
				Character->GetMesh1P()->GetAnimInstance()->Montage_JumpToSection("ReturnPose", ArmReloadMontage);
				if(WeaponReloadMontage != nullptr && FPWeaponMesh->HasValidAnimationInstance())
					FPWeaponMesh->GetAnimInstance()->Montage_JumpToSection("ReturnPose", WeaponReloadMontage);
				Character->IsReloading = false;
				UE_LOG(LogTemp, Log, TEXT("ResolveReload: %s"), Character->IsReloading ? TEXT("true") : TEXT("false"));
			}
		}
		else if(ReloadType == EReloadType::WholeAtOnce)
		{
			if(Character->AmmoMap[WeaponName] >= MagazineLimit)
			{
				Character->AmmoMap[WeaponName] -= MagazineLimit - CurrentAmmo;
				CurrentAmmo = MagazineLimit;
			}
			else
			{
				CurrentAmmo = Character->AmmoMap[WeaponName];
				Character->AmmoMap[WeaponName] = 0;
			}
				
			Character->IsReloading = false;
			UE_LOG(LogTemp, Log, TEXT("ResolveReload: %s"), Character->IsReloading ? TEXT("true") : TEXT("false"));
		}
		UE_LOG(LogTemp, Log, TEXT("Current Ammo : %d"), CurrentAmmo);
	}
}

void AWeaponBase::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == ArmFireMontage)
	{
		if(FireMode == EFireMode::Single)
		{
			IsFiring = false;
		}
	}
	else if((Montage == ArmReloadMontage || Montage == WeaponEmptyReloadMontage) && !bInterrupted)
	{
		OnActionCheck.Execute();
	}
}

bool AWeaponBase::Reload(UAnimInstance* CharacterAnimInstance)
{
	if(ArmReloadMontage != nullptr && CurrentAmmo < MagazineLimit
		&& !CharacterAnimInstance->Montage_IsPlaying(nullptr))
	{

		if(WeaponEmptyReloadMontage != nullptr && FPWeaponMesh->HasValidAnimationInstance() && CurrentAmmo == 0)
		{
			CharacterAnimInstance->Montage_Play(ArmEmptyReloadMontage);
			FPWeaponMesh->GetAnimInstance()->Montage_Play(WeaponEmptyReloadMontage);
		}
		else if(WeaponReloadMontage != nullptr && FPWeaponMesh->HasValidAnimationInstance())
		{
			CharacterAnimInstance->Montage_Play(ArmReloadMontage);
			FPWeaponMesh->GetAnimInstance()->Montage_Play(WeaponReloadMontage);
		}
		SoundManager->PlaySound(TEXT("Reload"), GetActorLocation());
		
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
	if(TargetCharacter == nullptr)
	{
		return;
	}
	
	FPWeaponMesh->SetSimulatePhysics(false);
	FPWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	FPWeaponMesh->AttachToComponent(TargetCharacter->GetMesh1P(), AttachmentRules, FPSocketName);
	
	TPWeaponMesh->SetVisibility(false);
	TPWeaponMesh->AttachToComponent(TargetCharacter->GetMesh(), AttachmentRules, TPSocketName);
	
	const int WeaponSlot = TargetCharacter->CollectedWeapon.Num();
	
	TargetCharacter->OnFire[WeaponSlot].Clear();
	TargetCharacter->OnFire[WeaponSlot].BindDynamic(this, &AWeaponBase::Fire);

	TargetCharacter->OnFireEnd[WeaponSlot].Clear();
	TargetCharacter->OnFireEnd[WeaponSlot].BindDynamic(this, &AWeaponBase::FireEnd);
	
	TargetCharacter->OnReload[WeaponSlot].Clear();
	TargetCharacter->OnReload[WeaponSlot].BindDynamic(this, &AWeaponBase::Reload);
	
	TargetCharacter->CollectedWeapon.Add(this);
	
	SetActorHiddenInGame(true); 
	SetActorEnableCollision(false); 
	SetActorTickEnabled(false);
	
	SoundManager->PlaySound("Equip", GetActorLocation());
	TargetCharacter->OnWeaponChange(TargetCharacter->CollectedWeapon.Num() - 1);
	
	TargetCharacter->GetMesh1P()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AWeaponBase::MontageEnded);

	if(TargetCharacter->AmmoMap.Contains(WeaponName))
	{
		TargetCharacter->AmmoMap[WeaponName] += InitialAmmo;
	}
	else
	{
		TargetCharacter->AmmoMap.Add(WeaponName, InitialAmmo);
	}
	
	OnActionCheck.BindDynamic(TargetCharacter, &AFPSurvivalCharacter::ActionCheck);
	PickUpComponent->OnComponentBeginOverlap.RemoveAll(PickUpComponent);
	PickUpComponent->OnComponentEndOverlap.RemoveAll(PickUpComponent);

	PickUpComponent->RemoveNearWeaponInfo(TargetCharacter);
	
	IsAttached = true;
}

void AWeaponBase::DetachWeapon(AFPSurvivalCharacter* TargetCharacter, const FTransform DetachTransform, const int DetachWeaponSlot)
{
	if(TargetCharacter == nullptr)
	{
		return;
	}

	FPWeaponMesh->SetSimulatePhysics(true);
	FPWeaponMesh->SetCollisionProfileName(TEXT("WeaponMesh"));
	
	const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, false);
	DetachFromActor(DetachmentRules);

	SetActorTransform(DetachTransform, true, nullptr, ETeleportType::TeleportPhysics);
	TargetCharacter->OnFire[DetachWeaponSlot].Clear();
	TargetCharacter->OnFireEnd[DetachWeaponSlot].Clear();
	TargetCharacter->OnReload[DetachWeaponSlot].Clear();

	TargetCharacter->CollectedWeapon.Remove(this);
	
	TargetCharacter->GetMesh1P()->GetAnimInstance()->OnMontageEnded.RemoveDynamic(this, &AWeaponBase::MontageEnded);

	TargetCharacter->AmmoMap[WeaponName] += CurrentAmmo;
	CurrentAmmo = 0;
	InitialAmmo = 0;
	
	OnActionCheck.Clear();

	PickUpComponent->RegisterOverlapFunction();
	PickUpComponent->AddNearWeaponInfo(TargetCharacter);

	IsAttached = false;
}