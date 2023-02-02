// Copyright Epic Games, Inc. All Rights Reserved.

#include "PickUpComponent.h"
#include "FPSurvivalCharacter.h"
#include "WeaponBase.h"

UPickUpComponent::UPickUpComponent()
{
	// Setup the Sphere Collision
	SphereRadius = 32.f;
	OnComponentBeginOverlap.AddDynamic(this, &UPickUpComponent::OnSphereBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UPickUpComponent::OnSphereEndOverlap);
}

void UPickUpComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPickUpComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checking if it is a First Person Character overlapping
	// if(Character != nullptr && Character->CollectedWeapon.Num() < 2)
	// {
	// 	// Notify that the actor is being picked up
	// 	OnPickUp.Broadcast(Character);
	// 	
	// 	// Unregister from the Overlap Event so it is no longer triggered
	// 	OnComponentBeginOverlap.RemoveAll(this);
	// }

	AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(OtherActor);
	if(Character != nullptr)
	{
		Character->NearWeapons.Add(Cast<AWeaponBase>(GetOwner()));
		UWidget* PickUpWidget = Character->HudWidget->GetWidgetFromName(TEXT("WBPickUp"));
		if(!PickUpWidget->IsVisible())
			PickUpWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPickUpComponent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(OtherActor);

	RemoveNearWeaponInfo(Character);
}

void UPickUpComponent::RemoveNearWeaponInfo(AFPSurvivalCharacter* Character)
{
	Character->NearWeapons.Remove(Cast<AWeaponBase>(GetOwner()));
	if(Character->NearWeapons.Num() == 0)
	{
		UWidget* PickUpWidget = Character->HudWidget->GetWidgetFromName(TEXT("WBPickUp"));
		if(PickUpWidget->IsVisible())
			PickUpWidget->SetVisibility(ESlateVisibility::Hidden);

		Character->NearestWeapon = nullptr;
	}
}
