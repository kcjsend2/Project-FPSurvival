// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PoolableActor.h"
#include "Components/ActorComponent.h"
#include "ActorObjectPoolComponent.generated.h"


UCLASS()
class FPSURVIVAL_API UActorObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActorObjectPoolComponent();

	UFUNCTION()
	APoolableActor* SpawnPooledActor();

	UPROPERTY(EditAnywhere)
	TSubclassOf<APoolableActor> PoolableActorSubclass;
	
	UPROPERTY(EditAnywhere)
	int InitialPoolSize;

	UFUNCTION()
	void OnPooledActorDespawn(APoolableActor* PooledActor);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TQueue<APoolableActor*> ObjectPool;
};
