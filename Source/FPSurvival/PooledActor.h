// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PooledActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPooledActorDespawn, class APooledActor*, PooledActor);

UCLASS()
class APooledActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APooledActor();
	
	FOnPooledActorDespawn OnPooledActorDespawn;
	
	UFUNCTION()
	void Deactivate();
	
	virtual void SetActive(bool Active);
	virtual bool IsActive();
	virtual void SetDefault() {}
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bIsActive;
};
