// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemPickup.h"
#include "PoolableCharacter.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class FPSURVIVAL_API AEnemyCharacter : public APoolableCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();
	
	void SetDropItem(AItemPickup* Item);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category="Item")
	AItemPickup* DropItem;
	
public:	
	virtual bool MeleeAttack() { return false; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float MaxHP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float CurrentHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackRange;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackDamage;
	
	bool IsAttacking = false;
};
