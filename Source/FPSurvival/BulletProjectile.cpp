// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletProjectile.h"

#include "FPSurvivalCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ABulletProjectile::ABulletProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetupAttachment(BulletMesh);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABulletProjectile::OnSphereBeginOverlap);
	
	SetRootComponent(BulletMesh);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

void ABulletProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(10.0f);
	ProjectileMovementComponent->InitialSpeed = 30000.0f;
}

void ABulletProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(OtherActor);
	if(Character != nullptr)
	{
		const FHitResult HitInfo;
		UGameplayStatics::ApplyPointDamage(Character, BulletDamage, LocationFired, HitInfo, GetInstigatorController(), this, nullptr);
	}
}

