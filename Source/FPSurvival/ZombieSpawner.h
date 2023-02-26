// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorObjectPoolComponent.h"
#include "CharacterObjectPoolComponent.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawner.generated.h"

UCLASS()
class FPSURVIVAL_API AZombieSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZombieSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UActorObjectPoolComponent* PistolAmmoObjectPool;
	
	UPROPERTY(EditAnywhere)
	UActorObjectPoolComponent* RifleAmmoObjectPool;
	
	UPROPERTY(EditAnywhere)
	UActorObjectPoolComponent* HealthPickupObjectPool;

	UPROPERTY(EditAnywhere)
	UCharacterObjectPoolComponent* ZombieObjectPool;
	
	UPROPERTY(EditAnywhere)
	TArray<AActor*> SpawnPoint;
	
	// 모두 합쳐 100이하여야 함.
	// 합치고 남은 값이 아무것도 드랍하지 않을 확률
	UPROPERTY(EditAnywhere)
	float RifleAmmoDropChance;
	
	UPROPERTY(EditAnywhere)
	float PistolAmmoDropChance;
	
	UPROPERTY(EditAnywhere)
	float HealthPickupDropChance;
	
	FRandomStream RandomStream;
	
	int AliveZombieCounter = 0;

	bool IsSpawning = false;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void SpawnZombieAtCurrentLocation();
	void SpawnZombie(FVector Location, FRotator Rotation);

	UFUNCTION()
	void SpawnZombieAtSpawnPoint(int ZombieNum, float Interval = 0.1f);

	UFUNCTION()
	void OnZombieDead();

	int GetZombieCounter() const { return AliveZombieCounter; }
	bool GetIsSpawning() const { return IsSpawning; }
};
