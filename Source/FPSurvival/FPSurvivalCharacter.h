// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CrossHairWidget.h"
#include "HudWidget.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MovementStateMachine.h"
#include "FPSurvivalCharacter.generated.h"

#define WEAPON_MAX 2

class AWeaponBase;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class UVaultingComponent;
class UWallRunningComponent;
class UPickUpWidget;
class UWidgetComponent;
class AFPSurvivalCharacter;

// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFire, AFPSurvivalCharacter*, Character);
DECLARE_DYNAMIC_DELEGATE(FOnFireEnd);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnReload, UAnimInstance*, CharacterAnimInstance);

UCLASS(config=Game)
class AFPSurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category="Character", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;
	
	/** First person camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

public:
	AFPSurvivalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Jump() override;
	
	UFUNCTION()
	bool WalkToCrouchTransition();
	
	UFUNCTION()
	bool WalkToSprintTransition();

	UFUNCTION()
	bool CrouchToWalkTransition();
	
	UFUNCTION()
    bool SprintToSlideTransition();

	UFUNCTION()
	bool SprintToWalkTransition();
    
    UFUNCTION()
	bool SlideToCrouchTransition();

	UFUNCTION()
	bool SlideToWalkTransition();

	UFUNCTION()
	bool SlideToSprintTransition();
	
	UFUNCTION()
	void SprintInit();

	UFUNCTION()
	void WalkInit();
	
	UFUNCTION()
	void SlideInit();
	
	UFUNCTION()
	void CrouchInit();

	UFUNCTION()
	void SprintEnd();

	UFUNCTION()
	void WalkEnd() {}
	
	UFUNCTION()
	void SlideEnd();
	
	UFUNCTION()
	void CrouchEnd();
	
	void SetStateMachineTransition();
	
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float TurnRateGamepad;

	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY()
	FOnFire OnFire[WEAPON_MAX];
	
	UPROPERTY()
	FOnFireEnd OnFireEnd[WEAPON_MAX];

	bool FireEndFlag = false;
	
	UPROPERTY()
	FOnReload OnReload[WEAPON_MAX];

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	UFUNCTION(BlueprintCallable)
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UPROPERTY(VisibleAnywhere, Category="UI")
	UCrossHairWidget* CrossHairWidget;

	
	
	UPROPERTY(VisibleAnywhere, Category="UI")
	UHudWidget* HudWidget;

	UPROPERTY()
	UPickUpWidget* PickUpWidget;
	
	FVector2d GetHorizontalVelocity() const { return FVector2d(GetCharacterMovement()->Velocity); }
	void SetHorizontalVelocity(float VelocityX, float VelocityY) const;
	void GainJumpCount() { JumpCurrentCount++; }
	
	int GetCurrentWeaponID() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsSprinting() const;
	
	float SprintMultiplier;
	float CrouchMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float MaxHP = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float MaxStamina = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float CurrentHP = 150;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float CurrentStamina = 150;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float StaminaRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float StaminaRegenCoolDown;

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
	
	TMap<EMovementState, float> SpeedMap;
	TMap<FName, bool> ButtonPressed;
	TMap<FName, int> AmmoMap;

	bool IsInSight = false;
	bool IsReloading = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsWeaponChanging = false;
	
	int CurrentWeaponSlot = -1;
	int ChangingWeaponSlot = -1;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* SmoothCrouchingCurveFloat;
	
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CameraTiltCurveFloat;
	
	FOnTimelineEvent SlideTimelineFunction;
	FOnTimelineFloat SmoothCrouchTimelineFunction;
	FOnTimelineFloat CameraTiltTimelineFunction;

	UPROPERTY()
	UTimelineComponent* RecoilTimeline;
	
	FOnTimelineFloat RecoilTimelineFunction;
	
	UPROPERTY(EditAnywhere, Category = "Recoil")
	UCurveFloat* RecoilCurveFloat;

	UFUNCTION()
	void RecoilTimelineReturn(float Value);
	
	UFUNCTION()
	void SmoothCrouchTimelineReturn(float Value);
	
	UFUNCTION()
	void CameraTiltReturn(float Value);
	
	UFUNCTION()
	void SlideTimelineReturn();

	UFUNCTION()
	void ActionCheck();

	UFUNCTION()
	void OnWeaponChangeEnd();

	UFUNCTION()
	void OnWeaponChange(int WeaponNum);

	UFUNCTION()
	void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);

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

	UPROPERTY(BlueprintReadOnly)
	TArray<AWeaponBase*> CollectedWeapon;
	
	UPROPERTY(BlueprintReadOnly, Category="Weapon")
	AWeaponBase* CurrentWeapon;

	UPROPERTY()
	AWeaponBase* NearestWeapon;
	
	UPROPERTY()
	TArray<AWeaponBase*> NearWeapons;

	// 1초 동안 누르면 무기 획득
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PickUpSpeed = 1;
	
protected:
	/** Fires a projectile. */
	void OnPrimaryAction(const bool Pressed);	
	void OnReloadAction(const bool Pressed);
	
	bool CanStand();
	bool CanSprint();

	void BeginSlide();
	void EndSlide();

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
	void OnSightAction(bool Pressed);
	void OnInteraction(bool Pressed);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	FVector CalculateFloorInfluence(FVector FloorNormal);
	TouchData	TouchItem;

	UPROPERTY()
	UMovementStateMachine* StateMachine;
	
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
};

