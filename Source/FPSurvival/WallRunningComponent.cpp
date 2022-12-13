// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunningComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UWallRunningComponent::UWallRunningComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	OwningCharacter = Cast<AFPSurvivalCharacter>(GetOwner());
}


// Called when the game starts
void UWallRunningComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UWallRunningComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ClampHorizontalVelocity();
}


void UWallRunningComponent::ClampHorizontalVelocity() const
{
	if(OwningCharacter->GetCharacterMovement()->IsFalling() /*&& OwningCharacter->GetMovementState() != EMovementState::Sliding*/)
	{
		const auto BaseVelocity = OwningCharacter->GetHorizontalVelocity().Length() / OwningCharacter->GetCharacterMovement()->GetMaxSpeed();
		if(BaseVelocity > 1.0f)
		{
			const auto ModifiedVelocity = OwningCharacter->GetHorizontalVelocity() / BaseVelocity;
			OwningCharacter->SetHorizontalVelocity(ModifiedVelocity.X, ModifiedVelocity.Y);
		}
	}
}
