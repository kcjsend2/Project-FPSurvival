// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CrossHairWidget.h"
#include "DeadMenuWidget.h"
#include "GameStateWidget.h"
#include "HudWidget.h"
#include "ItemPickup.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MovementStateMachine.h"
#include "SoundManager.h"
#include "WinMenuWidget.h"
#include "Components/SphereComponent.h"
#include "FPSurvivalCharacter.generated.h"

#define WEAPON_MAX 2

class USoundCue;
class AWeaponBase;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class UVaultingComponent;
class UPickUpWidget;
class UWidgetComponent;
class AFPSurvivalCharacter;
class UHitIndicator;

// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFire, AFPSurvivalCharacter*, Character)
DECLARE_DYNAMIC_DELEGATE(FOnFireEnd)
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnReload, UAnimInstance*, CharacterAnimInstance)
DECLARE_MULTICAST_DELEGATE(FOnDead)

UENUM()
enum class EWallRunningSide : uint8
{
	Left,
	Right
};

UENUM()
enum class EWallRunningEndReason : uint8
{
	FallOffWall,
	JumpOffWall
};

struct FWallRunningInfo
{
	EWallRunningSide Side;
	FVector Direction;
};

UCLASS(config=Game)
class AFPSurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category="Character", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;
	
	/** First person camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCameraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	USphereComponent* ItemHomingRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	USphereComponent* ItemPickupRange;
	
public:
	AFPSurvivalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Jump() override;
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	void SetItemHoming(AItemPickup* Item) const;

	UFUNCTION()
	void OnPlayerDisable();
	
	UFUNCTION()
	void OnItemHomingRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnItemPickupRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
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
	
	UFUNCTION()
	void SetStateMachineTransition();

	UFUNCTION()
	void ConsumeStamina(float StaminaConsume);

	UFUNCTION()
	void RegenStamina(float DeltaSeconds);
	
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float TurnRateGamepad;

	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY()
	FOnFire OnFire[WEAPON_MAX];
	
	UPROPERTY()
	FOnFireEnd OnFireEnd[WEAPON_MAX];

	bool FullAutoEndFlag = false;
	
	UPROPERTY()
	FOnReload OnReload[WEAPON_MAX];

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	UFUNCTION(BlueprintCallable)
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UPROPERTY(VisibleAnywhere, Category="UI")
	UCrossHairWidget* CrosshairWidget;

	UPROPERTY(VisibleAnywhere, Category="UI")
	UHudWidget* HudWidget;

	UPROPERTY(VisibleAnywhere, Category="UI")
	UPickUpWidget* PickUpWidget;
	
	UPROPERTY(VisibleAnywhere, Category="UI")
	UGameStateWidget* GameStateWidget;

	UPROPERTY(EditAnywhere, Category="Mesh")
	float TPMeshCrouchingZOffset;
	
	void GainJumpCount() { JumpCurrentCount++; }
	
	int GetCurrentWeaponID() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsSprinting() const;
	bool IsCrouching() const;

	float SprintMultiplier;
	float CrouchMultiplier;

	void OnWaveStart() const;
	void OnWaveReady(int CurrentWave);
	void SetMaxWaveInfo(int MaxWaveInfo);
	void SetWaveReadyRemainTime(FTimespan RemainTime);
	void SetWaveProgressRemainTime(FTimespan RemainTime);
	void SetZombieCounter(int ZombieCounter);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float MaxHP = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float MaxStamina = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float CurrentHP;
	float GetCurrentHP() const { return CurrentHP; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float CurrentStamina;

	// 초당 스태미나 회복
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float StaminaRegenValue;
	
	bool StaminaRegenHot = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float StaminaRegenInterval;

	float StaminaRegenCoolDown = 0.f;
	
	// 초당 스태미나 소모량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float SprintStaminaConsume;

	// 상태 진입 시 즉시 스태미나 소모량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float DoubleJumpStaminaConsume;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float SlideStaminaConsume;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	float WallRunningStaminaConsume;
	
	int CurrentJumpCount = 0;
	int MaxJumpCount = 0;

	float StandingCapsuleHalfHeight = 0;
	float StandingCameraZOffset = 0;

	float SlideFloorInfluence = 500000;
	
	float DefaultGroundFriction;
	float DefaultBrakingDeceleration;
	
	float SlideCoolDown = 0.0f;
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
	
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* RecoilCurveFloat;
	
	FOnTimelineEvent WallRunningTimelineFunction;
	FOnTimelineEvent SlideTimelineFunction;
	FOnTimelineFloat SmoothCrouchTimelineFunction;
	FOnTimelineFloat CameraTiltTimelineFunction;
	FOnTimelineFloat RecoilTimelineFunction;
	
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
	void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void DamageToOtherActor(bool Headshot, bool Dead, float Damage);

	UFUNCTION(BlueprintCallable)
	EMovementState GetCurrentMovementState() const { return StateMachine->GetCurrentState(); }
	
	UPROPERTY()
	UTimelineComponent* SlideTimeline;

	UPROPERTY()
	UTimelineComponent* SmoothCrouchingTimeline;
	
	UPROPERTY()
	UTimelineComponent* CameraTiltTimeline;
	
	UPROPERTY()
	UTimelineComponent* RecoilTimeline;
	
	UPROPERTY()
	UTimelineComponent* WallRunningTimeline;
	
	UPROPERTY(BlueprintReadWrite, Category="Vaulting")
	UVaultingComponent* VaultingComponent;

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

	UPROPERTY(EditAnywhere)
	USoundManager* SoundManager;

	UPROPERTY()
	UAudioComponent* AudioComponent;

	FOnDead OnDead;
	
protected:
	/** Fires a projectile. */
	void OnPrimaryAction(const bool Pressed);	
	void OnReloadAction(const bool Pressed);
	
	bool CanStand();
	bool CanSprint();

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
	
	void OnSprintAction(bool Pressed);
	void OnCrouchAction(bool Pressed);
	void OnSightAction(bool Pressed);
	void OnInteraction(bool Pressed);
	void OnDamageTest(bool Pressed);
	
	FVector CalculateFloorInfluence(FVector FloorNormal);

	UPROPERTY()
	UMovementStateMachine* StateMachine;
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface
	
	void BeginWallRunning();
	void EndWallRunning(EWallRunningEndReason EndReason);

	
	FWallRunningInfo FindWallRunningDirectionAndSide(FVector WallNormal) const;
	bool CanSurfaceBeWallRan(FVector SurfaceNormal) const;
	FVector FindLaunchDirection() const;
	bool IsWallRunningKeysDown() const;
	void ClampHorizontalVelocity() const;
	void SetHorizontalVelocity(float VelocityX, float VelocityY) const;
	
	FVector2d GetHorizontalVelocity() const { return FVector2d(GetCharacterMovement()->Velocity); }

	UFUNCTION()
	void UpdateWallRunning();
	
	// 벽 타기 관련 변수들
	UPROPERTY()
	FVector WallRunningDirection;
	
	UPROPERTY()
	bool IsWallRunning = false;
	
	UPROPERTY()
	bool WallRunningHot = false;
	
	UPROPERTY()
	float RightAxis;
	
	UPROPERTY()
	float ForwardAxis;
	
	UPROPERTY()
	EWallRunningSide WallRunningSide;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCrossHairWidget> CrossHairWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UHudWidget> HudWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UHitIndicator> HitIndicatorClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameStateWidget> GameStateWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UDeadMenuWidget> DeadMenuWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UWinMenuWidget> WinMenuWidgetClass;
	
	UPROPERTY()
	FVector MeshDefaultRelativeLocation;

	bool bIsDead = false;
};

