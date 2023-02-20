// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PoolableCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoolableCharacterDespawn, class APoolableCharacter*, PoolalbeCharacter);

UCLASS()
class FPSURVIVAL_API APoolableCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APoolableCharacter();
	
	FOnPoolableCharacterDespawn OnPoolableActorDespawn;
	
	UFUNCTION()
	void Deactivate();
	
	virtual void SetActive(bool Active);
	virtual bool IsActive();
	virtual void SetDefault() {}
	
protected:
	bool bIsActive;
};
