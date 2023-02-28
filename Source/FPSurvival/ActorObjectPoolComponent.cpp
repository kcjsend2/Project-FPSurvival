// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorObjectPoolComponent.h"

// Sets default values for this component's properties
UActorObjectPoolComponent::UActorObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UActorObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
	if(PoolableActorSubclass != nullptr)
	{
		for(int i = 0; i < InitialPoolSize; ++i)
		{
			ObjectPool.Push(LoadPoolableActor());
		}
	}
}

APoolableActor* UActorObjectPoolComponent::LoadPoolableActor()
{
	UWorld* const World = GetWorld();
	APoolableActor* SpawnedActor = World->SpawnActor<APoolableActor>(PoolableActorSubclass, FVector().ZeroVector, FRotator().ZeroRotator);
	if(SpawnedActor != nullptr)
	{
		SpawnedActor->SetActive(false);
		SpawnedActor->SetDefault();
		SpawnedActor->OnPoolableActorDespawn.AddDynamic(this, &UActorObjectPoolComponent::OnPoolableActorDespawn);
	}
	return SpawnedActor;
}

void UActorObjectPoolComponent::OnPoolableActorDespawn(APoolableActor* PoolableActor)
{
	PoolableActor->SetActorLocation(FVector().ZeroVector);
	PoolableActor->SetActorRotation(FRotator().ZeroRotator);
	PoolableActor->SetDefault();
	ObjectPool.Push(PoolableActor);
}

APoolableActor* UActorObjectPoolComponent::SpawnPoolableActor()
{
	APoolableActor* PoolableActor;
	if(ObjectPool.Num() == 0)
	{
		PoolableActor = LoadPoolableActor();
		PoolableActor->SetDefault();
		return PoolableActor;
	}
	
	PoolableActor = ObjectPool.Last();
	if(PoolableActor != nullptr)
	{
		ObjectPool.Remove(PoolableActor);
		PoolableActor->SetActive(true);
		PoolableActor->SetDefault();
		return PoolableActor;
	}
	return nullptr;
}