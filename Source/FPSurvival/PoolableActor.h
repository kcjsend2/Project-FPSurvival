// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoolableActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoolableActorDespawn, class APoolableActor*, PoolableActor);

UCLASS()
class FPSURVIVAL_API APoolableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APoolableActor();
	
	FOnPoolableActorDespawn OnPoolableActorDespawn;
	
	UFUNCTION()
	void Deactivate();
	
	virtual void SetActive(bool Active);
	virtual bool IsActive();
	virtual void SetDefault() {}
	
protected:
	bool bIsActive;
};
