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
		UWorld* const World = GetWorld();
		for(int i = 0; i < InitialPoolSize; ++i)
		{
			APoolableActor* SpawnedActor = World->SpawnActor<APoolableActor>(PoolableActorSubclass, FVector().ZeroVector, FRotator().ZeroRotator);
			if(SpawnedActor != nullptr)
			{
				SpawnedActor->SetActive(false);
				SpawnedActor->OnPoolableActorDespawn.AddDynamic(this, &UActorObjectPoolComponent::OnPooledActorDespawn);
				ObjectPool.Enqueue(SpawnedActor);
			}
		}
	}
}

void UActorObjectPoolComponent::OnPooledActorDespawn(APoolableActor* PooledActor)
{
    ObjectPool.Enqueue(PooledActor);
}

APoolableActor* UActorObjectPoolComponent::SpawnPooledActor()
{
	APoolableActor* PoolableActor;
	
	if(ObjectPool.Dequeue(PoolableActor))
	{
		PoolableActor->SetActive(true);
		PoolableActor->SetDefault();
		return PoolableActor;
	}

	UWorld* const World = GetWorld();
	APoolableActor* SpawnedActor = World->SpawnActor<APoolableActor>(PoolableActorSubclass, FVector().ZeroVector, FRotator().ZeroRotator);
	if(SpawnedActor != nullptr)
	{
		SpawnedActor->SetActive(true);
		SpawnedActor->OnPoolableActorDespawn.AddDynamic(this, &UActorObjectPoolComponent::OnPooledActorDespawn);
		SpawnedActor->SetDefault();
		return SpawnedActor;
	}

	return nullptr;
}