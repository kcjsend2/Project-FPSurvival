// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FPSurvivalCharacter.generated.h"

class UTP_WeaponComponent;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class UVaultingComponent;
class UWallRunningComponent;

// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUseItem);

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Walking,
	Sprinting,
	Crouching,
	Sliding
};

UCLASS(config=Game)
class AFPSurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

public:
	AFPSurvivalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Jump() override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float TurnRateGamepad;

	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnUseItem OnUseItem;

	float SprintMultiplier;
	float CrouchMultiplier;

	int CurrentJumpCount = 0;
	int MaxJumpCount = 0;

	float StandingCapsuleHalfHeight = 0;
	float StandingCameraZOffset = 0;

	float SlideFloorInfluence = 500000;
	
	float DefaultGroundFriction;
	float DefaultBrakingDeceleration;

	float SlideCoolTime = 0.0f;
	float SlideGroundFriction = 0;
	bool SlideHot = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sliding")
	float SlideInterval = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sliding")
	float SlideBrakingDeceleration = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sliding")
	float SlidePower = 5;
	
	float GroundSmashForce = -5000;
	
	TMap<EMovementState, float> SpeedMap;
	TMap<FName, bool> ButtonPressed;
	
	EMovementState MovementState;
	EMovementState PrevMovementState;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* SmoothCrouchingCurveFloat;
	
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CameraTiltCurveFloat;
	
	FOnTimelineEvent SlideTimelineFunction;
	FOnTimelineFloat SmoothCrouchTimelineFunction;
	FOnTimelineFloat CameraTiltTimelineFunction;

	UFUNCTION()
	void SmoothCrouchTimelineReturn(float Value);
	
	UFUNCTION()
	void CameraTiltReturn(float Value);
	
	UFUNCTION()
	void SlideTimelineReturn();

	UPROPERTY()
	UTimelineComponent* SlideTimeline;

	UPROPERTY()
	UTimelineComponent* SmoothCrouchingTimeline;
	
	UPROPERTY()
	UTimelineComponent* CameraTiltTimeline;

	UPROPERTY(BlueprintReadWrite, Category="Vaulting")
	UVaultingComponent* VaultingComponent;

	UPROPERTY(BlueprintReadWrite, Category="WallRunning")
	UWallRunningComponent* WallRunningComponent;

	UPROPERTY()
	TArray<UTP_WeaponComponent*> CollectedWeapon;
	
	UPROPERTY(BlueprintReadOnly, Category="Weapon")
	UTP_WeaponComponent* CurrentWeapon;
	
protected:
	/** Fires a projectile. */
	void OnPrimaryAction();	

	bool CanStand();
	bool CanSprint();

	void BeginSlide();
	void EndSlide();

	void SetMovementState(EMovementState NewMovementState);
	void ResolveMovementState();
	void OnMovementStateChanged();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void OnSprintAction(bool Pressed);
	void OnCrouchAction(bool Pressed);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	FVector CalculateFloorInfluence(FVector FloorNormal);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);
	
public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	FVector2d GetHorizontalVelocity() const { return FVector2d(GetCharacterMovement()->Velocity); }
	void SetHorizontalVelocity(float VelocityX, float VelocityY) const;
	void GainJumpCount() { JumpCurrentCount++; }
	
	EMovementState GetMovementState() { return MovementState; }
	int GetCurrentWeaponID() const;
};

