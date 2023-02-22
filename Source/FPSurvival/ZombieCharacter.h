// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

UCLASS()
class FPSURVIVAL_API AZombieCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

	UPROPERTY()
	USphereComponent* MeleeAttackSphere;
	
public:
	// Sets default values for this character's properties
	AZombieCharacter();

	UPROPERTY(EditAnywhere)
	float DefaultWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float SprintWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float DeactivateCoolDown = 3.0f;

	virtual void SetDefault() override;
	virtual void SetActive(bool Active) override;
	
	void BeginWalk();
	void BeginSprint();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION()
	void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	FTransform DefaultMeshRelativeTransform;
	
public:	
	virtual bool MeleeAttack() override;
};