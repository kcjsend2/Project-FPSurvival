// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PoolableCharacter.h"
#include "Components/ActorComponent.h"
#include "CharacterObjectPoolComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSURVIVAL_API UCharacterObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterObjectPoolComponent();

	UFUNCTION()
	APoolableCharacter* SpawnPoolableCharacter();

	UPROPERTY(EditAnywhere)
	TSubclassOf<APoolableCharacter> PoolableCharacterSubclass;
	
	UPROPERTY(EditAnywhere)
	int InitialPoolSize;

	UFUNCTION()
	void OnPooledActorDespawn(APoolableCharacter* PooledActor);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TQueue<APoolableCharacter*> ObjectPool;
};
