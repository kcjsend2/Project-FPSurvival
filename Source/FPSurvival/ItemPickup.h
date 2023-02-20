// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "ItemPickup.generated.h"

class UProjectileMovementComponent;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Health,
	Ammo,
};

UCLASS()
class AItemPickup : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AItemPickup();

	void SetHomingTarget(USceneComponent* Target);
	void Activate();
	void Deactivate();
	UBoxComponent* GetBoxComponent() const { return PhysicsBoxComponent; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* PhysicsBoxComponent;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComponent;
	
public:
	UPROPERTY(EditAnywhere)
	FVector ActivateImpulse;
	
	bool IsHoming = false;

	EItemType ItemType;

	UPROPERTY(EditAnywhere)
	int Amount;
};

UCLASS()
class AHealthPickup : public AItemPickup
{
	GENERATED_BODY()

public:
	AHealthPickup();
};

UCLASS()
class AAmmoPickup : public AItemPickup
{
	GENERATED_BODY()
	
public:
	AAmmoPickup();

	UPROPERTY(EditAnywhere)
	FName AmmoName;
};