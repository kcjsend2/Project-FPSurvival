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

void AItemPickup::SetActive(bool Active)
{
	bIsActive = Active;
	
	SetActorEnableCollision(Active);
	SetActorTickEnabled(Active);
	SetActorHiddenInGame(!Active);
}

void AItemPickup::SetDefault()
{
	Super::SetDefault();
	
	ProjectileMovementComponent->bIsHomingProjectile = false;
	ProjectileMovementComponent->HomingTargetComponent = nullptr;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 0.f;
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
	
	ProjectileMovementComponent->Deactivate();
	ProjectileMovementComponent->SetAutoActivate(false);
	DeactivateItem();
}

// Called when the game starts or when spawned
void AItemPickup::BeginPlay()
{
	Super::BeginPlay();
	SetDefault();
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

void AItemPickup::ActivateItem()
{
	SetActorHiddenInGame(false);
	PhysicsBoxComponent->SetCollisionProfileName(TEXT("ItemDropped"));
	PhysicsBoxComponent->SetSimulatePhysics(true);
	PhysicsBoxComponent->AddImpulse(ActivateImpulse, NAME_None, true);
}

void AItemPickup::DeactivateItem()
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