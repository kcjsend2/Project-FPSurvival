// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class FPSURVIVAL_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:	
	virtual bool MeleeAttack() { return false; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float MaxHP = 200;
	
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
