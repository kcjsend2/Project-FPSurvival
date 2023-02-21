// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterObjectPoolComponent.h"

// Sets default values for this component's properties
UCharacterObjectPoolComponent::UCharacterObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts
void UCharacterObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
	if(PoolableCharacterSubclass != nullptr)
	{
		UWorld* const World = GetWorld();
		for(int i = 0; i < InitialPoolSize; ++i)
		{
			APoolableCharacter* SpawnedCharacter = World->SpawnActor<APoolableCharacter>(PoolableCharacterSubclass, FVector().ZeroVector, FRotator().ZeroRotator);
			if(SpawnedCharacter != nullptr)
			{
				SpawnedCharacter->SetActive(false);
				SpawnedCharacter->OnPoolableActorDespawn.AddDynamic(this, &UCharacterObjectPoolComponent::OnPooledActorDespawn);
				ObjectPool.Enqueue(SpawnedCharacter);
			}
		}
	}

}

void UCharacterObjectPoolComponent::OnPooledActorDespawn(APoolableCharacter* PooledActor)
{
	ObjectPool.Enqueue(PooledActor);
}

APoolableCharacter* UCharacterObjectPoolComponent::SpawnPoolableCharacter()
{
	APoolableCharacter* PoolableCharacter;
	
	if(ObjectPool.Dequeue(PoolableCharacter))
	{
		PoolableCharacter->SetActive(true);
		PoolableCharacter->SetDefault();
		return PoolableCharacter;
	}

	UWorld* const World = GetWorld();
	APoolableCharacter* SpawnedCharacter = World->SpawnActor<APoolableCharacter>(PoolableCharacterSubclass, FVector().ZeroVector, FRotator().ZeroRotator);
	if(SpawnedCharacter != nullptr)
	{
		SpawnedCharacter->SetActive(true);
		SpawnedCharacter->OnPoolableActorDespawn.AddDynamic(this, &UCharacterObjectPoolComponent::OnPooledActorDespawn);
		SpawnedCharacter->SetDefault();
		return SpawnedCharacter;
	}

	return nullptr;
}

