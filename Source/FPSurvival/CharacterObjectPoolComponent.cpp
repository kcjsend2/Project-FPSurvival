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
		SpawnedCharacter->OnPoolableActorDespawn.AddDynamic(this, &UCharacterObjectPoolComponent::OnPoolableCharacterDespawn);
	}
	return SpawnedCharacter;
}

void UCharacterObjectPoolComponent::OnPoolableCharacterDespawn(APoolableCharacter* PoolableCharacter)
{
	PoolableCharacter->SetActorLocation(FVector().ZeroVector);
	PoolableCharacter->SetActorRotation(FRotator().ZeroRotator);
	PoolableCharacter->SetDefault();
	ObjectPool.Push(PoolableCharacter);
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

