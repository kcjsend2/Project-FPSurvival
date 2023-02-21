// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolableActor.h"

// Sets default values
APoolableActor::APoolableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void APoolableActor::Deactivate()
{
	SetActive(false);
	OnPoolableActorDespawn.Broadcast(this);
}

void APoolableActor::SetActive(bool Active)
{
	bIsActive = Active;

	SetActorEnableCollision(Active);
	SetActorTickEnabled(Active);
	SetActorHiddenInGame(!Active);
}

bool APoolableActor::IsActive()
{
	return bIsActive;
}