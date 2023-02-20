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

	if(PooledActorSubclass != nullptr)
	{
		UWorld* const World = GetWorld();
		for(int i = 0; i < PoolSize; ++i)
		{
			APooledActor* SpawnedActor = World->SpawnActor<APooledActor>(PooledActorSubclass, FVector().ZeroVector, FRotator().ZeroRotator);
			if(SpawnedActor != nullptr)
			{
				SpawnedActor->SetActive(false);
				SpawnedActor->OnPooledActorDespawn.AddDynamic(this, &UActorObjectPoolComponent::OnPooledActorDespawn);
				ObjectPool.Enqueue(SpawnedActor);
			}
		}
	}
}

void UActorObjectPoolComponent::OnPooledActorDespawn(APooledActor* PooledActor)
{
}


APooledActor* UActorObjectPoolComponent::SpawnPooledActor()
{
}