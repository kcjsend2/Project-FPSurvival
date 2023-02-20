// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemPickup.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AItemPickup::AItemPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	PhysicsBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsBoxComponent"));

	SetRootComponent(PhysicsBoxComponent);
	ItemMesh->SetupAttachment(PhysicsBoxComponent);
}

// Called when the game starts or when spawned
void AItemPickup::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMovementComponent->Deactivate();
	ProjectileMovementComponent->SetAutoActivate(false);
	Deactivate();
}

void AItemPickup::SetHomingTarget(USceneComponent* Target)
{
	IsHoming = true;
	
	PhysicsBoxComponent->SetSimulatePhysics(false);
	PhysicsBoxComponent->SetCollisionProfileName(TEXT("ItemHoming"));
	
	ProjectileMovementComponent->Activate();
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingTargetComponent = Target;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 50000.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AItemPickup::Activate()
{
	SetActorHiddenInGame(false);
	PhysicsBoxComponent->SetCollisionProfileName(TEXT("ItemDropped"));
	PhysicsBoxComponent->SetSimulatePhysics(true);
	PhysicsBoxComponent->AddImpulse(ActivateImpulse, NAME_None, true);
}

void AItemPickup::Deactivate()
{
	SetActorHiddenInGame(true);
	PhysicsBoxComponent->SetCollisionProfileName(TEXT("NoCollsion"));
	PhysicsBoxComponent->SetSimulatePhysics(false);
}

AHealthPickup::AHealthPickup()
{
	ItemType = EItemType::Health;
}

AAmmoPickup::AAmmoPickup()
{
	ItemType = EItemType::Ammo;
}