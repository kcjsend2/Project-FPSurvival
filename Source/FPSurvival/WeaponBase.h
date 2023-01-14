// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BulletProjectile.h"
#include "WeaponBase.generated.h"

class UCameraComponent;
class AFPSurvivalCharacter;
class UPickUpComponent;

UCLASS()
class FPSURVIVAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	
	/** Sets default values for this component's properties */
	AWeaponBase();
	
	virtual void BeginPlay() override;

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void AttachWeapon(AFPSurvivalCharacter* TargetCharacter);

	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void Fire();
	
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void FireMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void Reload();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mesh)
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mesh)
	FVector WeaponRelativePosition;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mesh)
	FRotator WeaponRelativeRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int WeaponID;
	
	UPROPERTY(BlueprintReadOnly, Category="Weapon")
	bool IsFiring;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    bool IsAttached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int MagazineLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* ArmShootingMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* WeaponShootingMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* ArmReloadMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* WeaponReloadMontage;
	
protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	

private:
	/** The Character holding this weapon*/
	UPROPERTY()
	AFPSurvivalCharacter* Character;

	UPROPERTY(VisibleDefaultsOnly, Category="Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Weapon", meta = (AllowPrivateAccess = "true"))
	UPickUpComponent* PickUpComponent;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Weapon", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Muzzle;

	TSubclassOf<ABulletProjectile> BulletProjectileClass;
public:
	USkeletalMeshComponent* GetMesh() const { return WeaponMesh; }
};
