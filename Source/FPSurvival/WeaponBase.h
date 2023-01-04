// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.generated.h"

class AFPSurvivalCharacter;
class UPickUpComponent;

UCLASS()
class FPSURVIVAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AFPSurvivalProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

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
	void FireEnd();

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
	
protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	

private:
	/** The Character holding this weapon*/
	UPROPERTY()
	AFPSurvivalCharacter* Character;

	UPROPERTY(VisibleDefaultsOnly, Category="Weapon", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, Category="Weapon", meta = (AllowPrivateAccess = "true"))
	UPickUpComponent* PickUpComponent;
};
