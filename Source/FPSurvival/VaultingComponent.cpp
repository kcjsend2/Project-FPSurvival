// Fill out your copyright notice in the Description page of Project Settings.


#include "VaultingComponent.h"

#include "VectorTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UVaultingComponent::UVaultingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UVaultingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AFPSurvivalCharacter>(GetOwner());
}

FVaultingInfo UVaultingComponent::CheckCanVault() const
{
	if(IsVaulting)
	{
		return FVaultingInfo{false, FVector(0, 0, 0)};
	}

	const auto CapsuleComponent = OwningCharacter->GetCapsuleComponent();

	constexpr float VaultingDistance = 70.0f;
	
	FHitResult HitResult;
	FVector LineBase = OwningCharacter->GetActorLocation() + OwningCharacter->GetActorForwardVector() * VaultingDistance;
	
	FVector LineStart = LineBase;
	FVector LineEnd = LineBase;
	
	LineStart.Z += OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	LineEnd.Z -= OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwningCharacter);
	
	/*FName TraceTag = "Vaulting Trace";
	GetWorld()->DebugDrawTraceTag = TraceTag;
	
	CollisionParams.TraceTag = TraceTag;*/
	
	GetWorld()->LineTraceSingleByChannel(HitResult, LineStart, LineEnd, ECC_GameTraceChannel2, CollisionParams);

	
	if(HitResult.bBlockingHit)
	{
		auto Result =  CanVaultToHit(HitResult);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("CanVault : %s"), Result.bCanVault ? TEXT("true") : TEXT("false")));
		};

		return Result;
	}
	return FVaultingInfo();
}

FVaultingInfo UVaultingComponent::CanVaultToHit(const FHitResult HitResult) const
{
	const float VaultingHeight = HitResult.Location.Z - HitResult.TraceEnd.Z;

	const auto CapsuleRadius = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const auto CapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::SanitizeFloat(VaultingHeight));
	}
	
	if(!(VaultingHeight >= MinVaultingHeight && VaultingHeight <= MaxVaultingHeight))
	{
		return FVaultingInfo();
	}

	if(HitResult.Normal.Z < OwningCharacter->GetCharacterMovement()->GetWalkableFloorZ())
	{
		return FVaultingInfo();
	}

	const FVector EndingLocation = HitResult.Location + FVector(0, 0, CapsuleHalfHeight);

	const auto Center = EndingLocation + FVector(0, 0, CapsuleRadius);

	if(CheckCapsuleCollision(Center, CapsuleHalfHeight, CapsuleRadius, false))
	{
		return FVaultingInfo();
	}
	
	return FVaultingInfo{true, EndingLocation};
}

bool UVaultingComponent::CheckCapsuleCollision(const FVector Center, const float HalfHeight, const float Radius, const bool DebugDraw) const
{
	if(DebugDraw)
	{
		DrawDebugCapsule(GetWorld(), Center, HalfHeight, Radius, FQuat(FRotator(0, 0, 0)), FColor::Yellow, false, -1, 0, 5);
	}
	
	const auto CollisionShape = OwningCharacter->GetCapsuleComponent()->GetCollisionShape();

	TArray<FOverlapResult> OverlapResults;
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwningCharacter);

	FCollisionObjectQueryParams CollisionObjectQueryParams;
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_Destructible);
	
	return GetWorld()->OverlapMultiByObjectType(OverlapResults, Center, FQuat(FRotator(0)), CollisionObjectQueryParams, CollisionShape, CollisionParams);
}

void UVaultingComponent::Vault()
{
	VaultProgress = 0;
	VaultStartingLocation = OwningCharacter->GetActorLocation();
	IsVaulting = true;
}

// Called every frame
void UVaultingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!IsVaulting)
	{
		const auto [bCanVault, EndingLocation] = CheckCanVault();
		
		CanVault = bCanVault;
		if(bCanVault)
		{
			VaultEndingLocation = EndingLocation;
		}
	}
	else
	{
		TickVault(DeltaTime);
	}
}

void UVaultingComponent::TickVault(float DeltaTime)
{
	VaultProgress = FMath::Clamp(VaultProgress + DeltaTime / VaultSpeed, 0, 1);
	OwningCharacter->SetActorLocation(FMath::Lerp(VaultStartingLocation, VaultEndingLocation, VaultProgress));

	if(VaultProgress >= 1)
	{
		IsVaulting = false;
	}
}

