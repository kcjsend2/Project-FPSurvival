// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BulletProjectile.h"
#include "WeaponBase.generated.h"

class UCameraComponent;
class AFPSurvivalCharacter;
class UPickUpComponent;

DECLARE_DYNAMIC_DELEGATE(FOnActionCheck);

UENUM(BlueprintType)
enum class EReloadType : uint8
{
	WholeAtOnce,
	OneByOne
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single,
	FullAuto
};

UCLASS()
class FPSURVIVAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=UI)
	UTexture2D* WeaponImage;
	
	/** Sets default values for this component's properties */
	AWeaponBase();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION()
	void AttachWeapon(AFPSurvivalCharacter* TargetCharacter);
	
	UFUNCTION()
	void DetachWeapon(AFPSurvivalCharacter* TargetCharacter, const FTransform DetachTransform, const int DetachWeaponSlot);

	/** Make the weapon Fire a Projectile */
	UFUNCTION()
	void Fire(AFPSurvivalCharacter* Character);

	UFUNCTION()
	void FireEnd();

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; }
	
	UFUNCTION()
	void ResolveReload(bool bInterrupted, AFPSurvivalCharacter* Character);

	UFUNCTION()
	void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	bool Reload(UAnimInstance* CharacterAnimInstance);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mesh)
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mesh)
	FVector WeaponRelativePosition;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mesh)
	FRotator WeaponRelativeRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FName WeaponName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int InitialAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int WeaponID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int MagazineLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float BulletDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	EReloadType ReloadType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* ArmFireMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* ArmAimDownSightFireMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* WeaponFireMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* ArmReloadMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* WeaponReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* WeaponPullDownMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* WeaponPullUpMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	EFireMode FireMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FVector AimDownSightOffset;

	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilPitch;
	
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilPitchADS;
	
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilYaw;
	
	UPROPERTY(EditAnywhere, Category = "Recoil")
	bool RecoilOn;
	
	UPROPERTY(EditAnywhere, Category="UI")
	float SpreadPerShot;

	UPROPERTY(EditAnywhere, Category="UI")
	bool DotCrosshairInSight;

protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleDefaultsOnly, Category="Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Weapon", meta = (AllowPrivateAccess = "true"))
	UPickUpComponent* PickUpComponent;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Weapon", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Muzzle;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Weapon", meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* MuzzleFlash;
	
	TSubclassOf<ABulletProjectile> BulletProjectileClass;
	
	bool IsFiring = false;
	bool IsAttached = false;
	
	UPROPERTY()
	FOnActionCheck OnActionCheck;

public:
	USkeletalMeshComponent* GetMesh() const { return WeaponMesh; }
	bool GetIsFiring() const { return IsFiring; }
	bool GetIsAttached() const { return IsAttached; }
};
