// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolableCharacter.h"

// Sets default values
APoolableCharacter::APoolableCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void APoolableCharacter::Deactivate()
{
	SetActive(false);
	OnPoolableActorDespawn.Broadcast(this);
}

void APoolableCharacter::SetActive(bool Active)
{
	bIsActive = IsActive;
	SetActorHiddenInGame(!IsActive);
}

bool APoolableCharacter::IsActive()
{
	return bIsActive;
}