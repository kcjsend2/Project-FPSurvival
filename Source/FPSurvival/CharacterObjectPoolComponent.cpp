// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterObjectPoolComponent.h"

// Sets default values for this component's properties
UCharacterObjectPoolComponent::UCharacterObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCharacterObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
	if(PoolableCharacterSubclass != nullptr)
	{
		for(int i = 0; i < InitialPoolSize; ++i)
		{
			ObjectPool.Push(LoadPoolableCharacter());
		}
	}
}

APoolableCharacter* UCharacterObjectPoolComponent::LoadPoolableCharacter()
{
	UWorld* const World = GetWorld();
	APoolableCharacter* SpawnedCharacter = World->SpawnActor<APoolableCharacter>(PoolableCharacterSubclass, FVector().ZeroVector, FRotator().ZeroRotator);
	if(SpawnedCharacter != nullptr)
	{
		SpawnedCharacter->SetActive(false);
		SpawnedCharacter->SetDefault();
		SpawnedCharacter->OnPoolableActorDespawn.AddDynamic(this, &UCharacterObjectPoolComponent::OnPooledCharacterDespawn);
	}
	return SpawnedCharacter;
}

void UCharacterObjectPoolComponent::OnPooledCharacterDespawn(APoolableCharacter* PooledCharacter)
{
	PooledCharacter->SetActorLocation(FVector().ZeroVector);
	PooledCharacter->SetActorRotation(FRotator().ZeroRotator);
	PooledCharacter->SetDefault();
	ObjectPool.Push(PooledCharacter);
}

APoolableCharacter* UCharacterObjectPoolComponent::SpawnPoolableCharacter()
{
	APoolableCharacter* PoolableCharacter;
	if(ObjectPool.Num() == 0)
	{
		PoolableCharacter = LoadPoolableCharacter();
		if(PoolableCharacter == nullptr)
			return nullptr;
		
		PoolableCharacter->SetDefault();
		return PoolableCharacter;
	}
	
	PoolableCharacter = ObjectPool.Last();
	if(PoolableCharacter != nullptr)
	{
		ObjectPool.Remove(PoolableCharacter);
		PoolableCharacter->SetActive(true);
		PoolableCharacter->SetDefault();
		return PoolableCharacter;
	}
	return nullptr;
}

