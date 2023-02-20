// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	
	if(DropItem != nullptr)
	{
		const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		DropItem->AttachToActor(this, AttachmentRules);
	}
}

float AEnemyCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHP -= Damage;

	if(CurrentHP <= 0)
	{
		CurrentHP = 0;
		if(DropItem != nullptr)
		{
			const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, false);
			DropItem->DetachFromActor(DetachmentRules);
			DropItem->Activate();
			
			DropItem = nullptr;
			
		}
	}
	
	return Damage;
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

