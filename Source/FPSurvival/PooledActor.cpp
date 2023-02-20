// Fill out your copyright notice in the Description page of Project Settings.


#include "PooledActor.h"

// Sets default values
APooledActor::APooledActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void APooledActor::Deactivate()
{
	SetActive(false);
	OnPooledActorDespawn.Broadcast(this);
}

void APooledActor::SetActive(bool IsActive)
{
	bIsActive = IsActive;
	SetActorHiddenInGame(!IsActive);
}

bool APooledActor::IsActive()
{
	return bIsActive;
}

// Called when the game starts or when spawned
void APooledActor::BeginPlay()
{
	Super::BeginPlay();
	
}