// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PooledActor.h"
#include "Components/ActorComponent.h"
#include "ActorObjectPoolComponent.generated.h"


UCLASS()
class UActorObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActorObjectPoolComponent();

	UFUNCTION()
	APooledActor* SpawnPooledActor();

	UPROPERTY(EditAnywhere)
	TSubclassOf<APooledActor> PooledActorSubclass;
	
	UPROPERTY(EditAnywhere)
	int PoolSize;

	UFUNCTION()
	void OnPooledActorDespawn(APooledActor* PooledActor);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TQueue<APooledActor*> ObjectPool;
};
