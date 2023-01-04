// Copyright Epic Games, Inc. All Rights Reserved.

#include "PickUpComponent.h"

UPickUpComponent::UPickUpComponent()
{
	// Setup the Sphere Collision
	SphereRadius = 32.f;
	OnComponentBeginOverlap.AddDynamic(this, &UPickUpComponent::OnSphereBeginOverlap);
}

void UPickUpComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPickUpComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checking if it is a First Person Character overlapping
	AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(OtherActor);
	if(Character != nullptr && Character->CollectedWeapon.Num() < 2)
	{
		// Notify that the actor is being picked up
		OnPickUp.Broadcast(Character);
		
		// Unregister from the Overlap Event so it is no longer triggered
		OnComponentBeginOverlap.RemoveAll(this);
	}
}
