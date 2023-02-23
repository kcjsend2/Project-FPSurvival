// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemPickup.h"

#include "FPSurvivalCharacter.h"
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
	ProjectileMovementComponent->SetAutoActivate(false);
}

void AItemPickup::SetActive(bool Active)
{
	bIsActive = Active;

	if(!Active)
		DeactivateItem();
	
	SetActorEnableCollision(Active);
	SetActorTickEnabled(Active);
}

void AItemPickup::SetDefault()
{
	Super::SetDefault();

	DeactivateItem();
	
	IsHoming = false;
	
	ProjectileMovementComponent->bIsHomingProjectile = false;
	ProjectileMovementComponent->HomingTargetComponent = nullptr;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 0.f;
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
	
	ProjectileMovementComponent->Deactivate();
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

	TArray<AActor*> OverlappingActors;
	PhysicsBoxComponent->SetCollisionProfileName(TEXT("ItemHoming"));
	PhysicsBoxComponent->GetOverlappingActors(OverlappingActors, AFPSurvivalCharacter::StaticClass());
	
	if(OverlappingActors.Num() == 0)
	{
		PhysicsBoxComponent->SetCollisionProfileName(TEXT("ItemDropped"));
		PhysicsBoxComponent->SetSimulatePhysics(true);
		PhysicsBoxComponent->AddImpulse(ActivateImpulse, NAME_None, true);
	}
}

void AItemPickup::DeactivateItem()
{
	PhysicsBoxComponent->SetCollisionProfileName(TEXT("NoCollision"));
	PhysicsBoxComponent->SetSimulatePhysics(false);
	SetActorHiddenInGame(true);
}

AHealthPickup::AHealthPickup()
{
	ItemType = EItemType::Health;
}

AAmmoPickup::AAmmoPickup()
{
	ItemType = EItemType::Ammo;
}