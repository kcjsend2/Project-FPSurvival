// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPSurvivalCharacter.h"
#include "VaultingComponent.generated.h"

UENUM(BlueprintType)
enum class EVaultingState : uint8
{
	Idle,
	Vaulting
};

struct FVaultingInfo
{
	bool bCanVault;
	FVector EndingLocation;

	FVaultingInfo()
	{
		bCanVault = false;
		EndingLocation = FVector(0);
	}

	FVaultingInfo(const bool CanVault, const FVector Location)
	{
		bCanVault = CanVault;
		EndingLocation = Location;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSURVIVAL_API UVaultingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVaultingComponent();
	FVaultingInfo CheckCanVault() const;
	FVaultingInfo CanVaultToHit(FHitResult HitResult) const;
	bool CheckCapsuleCollision(FVector Center, float HalfHeight, float Radius, bool DebugDraw) const;
	void Vault();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly, Category="Vaulting")
	bool CanVault = false;

	UPROPERTY(BlueprintReadOnly, Category="Vaulting")
	EVaultingState VaultingState = EVaultingState::Idle;
	
private:
	void TickVault(float DeltaTime);
	
	UPROPERTY()
	AFPSurvivalCharacter* OwningCharacter;

	FVector VaultEndingLocation;
	FVector VaultStartingLocation;

	float MinVaultingHeight = 50;
	float MaxVaultingHeight = 170;

	float VaultProgress = 0;
	float VaultSpeed = 0.2;
};
