// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSurvivalCharacter.h"
#include "FPSurvivalProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VaultingComponent.h"
#include "Components/WidgetComponent.h"
#include "CrossHairWidget.h"
#include "DeadMenuWidget.h"
#include "FPSurvivalGameMode.h"
#include "FPSurvivalPlayerState.h"
#include "PickUpWidget.h"
#include "WeaponBase.h"
#include "HitIndicator.h"
#include "ItemPickup.h"
#include "Components/AudioComponent.h"
#include "Engine/DamageEvents.h"


AFPSurvivalCharacter::AFPSurvivalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 1.752765f, 70.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	ThirdPersonCameraComponent->SetRelativeLocation(FVector(-30.f, 10.f, 0.f));
	ThirdPersonCameraComponent->bUsePawnControlRotation = true;
	
	StateMachine = CreateDefaultSubobject<UMovementStateMachine>(TEXT("StateMachine"));
	SoundManager = CreateDefaultSubobject<USoundManager>(TEXT("SoundManager"));
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(GetCapsuleComponent());

	ItemHomingRange = CreateDefaultSubobject<USphereComponent>(TEXT("ItemHomingRange"));
	ItemHomingRange->SetupAttachment(GetCapsuleComponent());
	
	ItemPickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("ItemPickupRange"));
	ItemPickupRange->SetupAttachment(GetCapsuleComponent());
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(0.0f, -1.752765f, -160.0f));
	Mesh1P->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	
	SprintMultiplier = 1.7f;
	CrouchMultiplier = 0.6f;
	
	SpeedMap.Add(EMovementState::Walking, GetCharacterMovement()->MaxWalkSpeed);
	SpeedMap.Add(EMovementState::Sprinting, GetCharacterMovement()->MaxWalkSpeed * SprintMultiplier);
	SpeedMap.Add(EMovementState::Crouching, GetCharacterMovement()->MaxWalkSpeed * CrouchMultiplier);
	SpeedMap.Add(EMovementState::Sliding, NULL);
	
	ButtonPressed.Add("Sprint", false);
	ButtonPressed.Add("Crouch", false);
	ButtonPressed.Add("Sight", false);
	ButtonPressed.Add("Interaction", false);
	
	CurrentJumpCount = 0;
	MaxJumpCount = 2;

	SetStateMachineTransition();
	
	SmoothCrouchingTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SmoothCrouchingTimeline"));
	SlideTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SlideTimeline"));
	CameraTiltTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TiltTimeline"));
	RecoilTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RecoilTimeline"));
	WallRunningTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WallRunningTimeline"));
	
	VaultingComponent = CreateDefaultSubobject<UVaultingComponent>(TEXT("VaultingObject"));
	
	StandingCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	StandingCameraZOffset = GetFirstPersonCameraComponent()->GetRelativeLocation().Z;

	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
	
	SlideCoolDown = SlideInterval;
	
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
}

void AFPSurvivalCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	MeshDefaultRelativeLocation = GetMesh()->GetRelativeLocation();
	
	//Mesh1P->SetOwnerNoSee(true);
	GetMesh()->SetOwnerNoSee(true);
	//GetMesh()->SetCastShadow(true);

	//FirstPersonCameraComponent->Deactivate();
	//ThirdPersonCameraComponent->Activate();

	ItemHomingRange->OnComponentBeginOverlap.AddDynamic(this, &AFPSurvivalCharacter::OnItemHomingRangeBeginOverlap);
	ItemPickupRange->OnComponentBeginOverlap.AddDynamic(this, &AFPSurvivalCharacter::OnItemPickupRangeBeginOverlap);
	SoundManager->SetAudioComponent(AudioComponent);
	
	if(IsPlayerControlled())
	{
		HudWidget = CreateWidget<UHudWidget>(GetWorld(), HudWidgetClass, TEXT("Hud"));
		if(HudWidget != nullptr)
			HudWidget->AddToViewport();
	
		CrosshairWidget = CreateWidget<UCrossHairWidget>(GetWorld(), CrossHairWidgetClass, TEXT("CrossHair"));
		if(CrosshairWidget != nullptr)
			CrosshairWidget->AddToViewport();
		
		GameStateWidget = CreateWidget<UGameStateWidget>(GetWorld(), GameStateWidgetClass, TEXT("GameState"));
		if(GameStateWidget != nullptr)
			GameStateWidget->AddToViewport();
		
		PickUpWidget = Cast<UPickUpWidget>(HudWidget->GetWidgetFromName(TEXT("WBPickUp")));

		UDeadMenuWidget* DeadMenuWidget = CreateWidget<UDeadMenuWidget>(GetWorld(), DeadMenuWidgetClass, TEXT("DeadMenu"));
		if(DeadMenuWidget != nullptr)
		{
			DeadMenuWidget->AddToViewport();
			DeadMenuWidget->SetVisibility(ESlateVisibility::Hidden);
			OnDead.AddUFunction(DeadMenuWidget, "SetVisible");
			AFPSurvivalGameMode* GameMode = Cast<AFPSurvivalGameMode>(UGameplayStatics::GetGameMode(this));
			if(GameMode != nullptr)
			{
				DeadMenuWidget->OnGameRestart.AddUFunction(GameMode, TEXT("RestartGame"));
				GameMode->OnWaveFailed.AddUFunction(DeadMenuWidget, TEXT("SetVisible"));
				GameMode->OnWaveFailed.AddUFunction(this, TEXT("OnPlayerDisable"));
			}
		}

		UWinMenuWidget* WinMenuWidget = CreateWidget<UWinMenuWidget>(GetWorld(), WinMenuWidgetClass, TEXT("WinMenu"));
		if(WinMenuWidget != nullptr)
		{
			WinMenuWidget->AddToViewport();
			WinMenuWidget->SetVisibility(ESlateVisibility::Hidden);
			const AFPSurvivalPlayerState* CurrentPlayerState = Cast<AFPSurvivalPlayerState>(GetPlayerState());
			AFPSurvivalGameMode* GameMode = Cast<AFPSurvivalGameMode>(UGameplayStatics::GetGameMode(this));

			if(GameMode != nullptr)
			{
				GameMode->OnGameWin.AddLambda([=]()
			{
				WinMenuWidget->SetKillCount(CurrentPlayerState->GetKillScore());
				
			});
				GameMode->OnGameWin.AddLambda([=]()
				{
					WinMenuWidget->SetDamageDealt(CurrentPlayerState->GetDamageDealt());
				
				});
				GameMode->OnGameWin.AddLambda([=]()
				{
					WinMenuWidget->SetDamageTaken(CurrentPlayerState->GetDamageTaken());
				
				});
				GameMode->OnGameWin.AddUFunction(WinMenuWidget, "SetVisible");
				GameMode->OnGameWin.AddUFunction(this, TEXT("OnPlayerDisable"));
			}
		}
	}
	
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AFPSurvivalCharacter::OnCapsuleComponentHit);
	
	CameraTiltTimelineFunction.BindUFunction(this, FName("CameraTiltReturn"));
	if(CameraTiltCurveFloat)
	{
		CameraTiltTimeline->AddInterpFloat(CameraTiltCurveFloat, CameraTiltTimelineFunction);
		CameraTiltTimeline->SetTimelineLength(0.3);
		CameraTiltTimeline->SetLooping(false);
	}
	
	SmoothCrouchTimelineFunction.BindUFunction(this, FName("SmoothCrouchTimelineReturn"));
	if(SmoothCrouchingCurveFloat)
	{
		SmoothCrouchingTimeline->AddInterpFloat(SmoothCrouchingCurveFloat, SmoothCrouchTimelineFunction);
		SmoothCrouchingTimeline->SetTimelineLength(0.3);
		SmoothCrouchingTimeline->SetLooping(false);
	}
	
	RecoilTimelineFunction.BindUFunction(this, FName("RecoilTimelineReturn"));
	if(RecoilCurveFloat)
	{
		RecoilTimeline->AddInterpFloat(RecoilCurveFloat, RecoilTimelineFunction);
		RecoilTimeline->SetTimelineLength(0.1f);
		RecoilTimeline->SetLooping(false);
	}
	
	SlideTimelineFunction.BindUFunction(this, FName("SlideTimelineReturn"));
	SlideTimeline->AddEvent(0, SlideTimelineFunction);
	SlideTimeline->SetTimelineLength(1.0);
	SlideTimeline->SetLooping(true);

	WallRunningTimelineFunction.BindUFunction(this, FName("UpdateWallRunning"));
	WallRunningTimeline->AddEvent(0, WallRunningTimelineFunction);
	WallRunningTimeline->SetTimelineLength(1.0);
	WallRunningTimeline->SetLooping(true);
	
	Mesh1P->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AFPSurvivalCharacter::MontageEnded);
}

void AFPSurvivalCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(IsPlayerControlled())
	{
		CrosshairWidget->Spread = FMath::GetMappedRangeValueClamped(FVector2D(0, 1000), FVector2D(5, 80), GetVelocity().Length() + CrosshairWidget->FireSpreadValue);
		HudWidget->HPPercentage = CurrentHP / MaxHP;
		HudWidget->StaminaPercentage = CurrentStamina / MaxStamina;

		if(CurrentWeapon != nullptr)
		{
			HudWidget->CurrentMagazine = FString::FromInt(CurrentWeapon->CurrentAmmo);
			HudWidget->TotalAmmo = FString::FromInt(AmmoMap[CurrentWeapon->WeaponName]);
		}
		else
		{
			HudWidget->CurrentMagazine = TEXT("--");
			HudWidget->TotalAmmo = TEXT("--");
		}
		
		if(NearWeapons.Num() > 0)
		{
			float MaxDist = 0;
			for(AWeaponBase* Weapon : NearWeapons)
			{
				const float Dist = GetDistanceTo(Weapon);
				if(Dist > MaxDist)
					MaxDist = Dist;

				if(NearestWeapon != Weapon)
				{
					NearestWeapon = Weapon;
					PickUpWidget->PickUpGauge = 0;
				}
			}
			if(NearestWeapon != nullptr)
			{
				PickUpWidget->PickUpWeaponImage = NearestWeapon->WeaponImage;
				PickUpWidget->PickUpWeaponName = NearestWeapon->WeaponName.ToString();
			}
		}

		if(NearestWeapon != nullptr)
		{
			if(ButtonPressed["interaction"] && PickUpWidget->PickUpGauge < 1.f)
			{
				PickUpWidget->PickUpGauge += PickUpSpeed * DeltaSeconds;
				if(PickUpWidget->PickUpGauge > 1.f)
					PickUpWidget->PickUpGauge = 1.f;
			}
			else if(!ButtonPressed["interaction"] && PickUpWidget->PickUpGauge > 0.f)
			{
				PickUpWidget->PickUpGauge -= PickUpSpeed * DeltaSeconds;
				if(PickUpWidget->PickUpGauge < 0.f)
					PickUpWidget->PickUpGauge = 0.f;
			}

			if(PickUpWidget->PickUpGauge >= 1.f)
			{
				if(CollectedWeapon.Num() < 2)
				{
					NearestWeapon->AttachWeapon(this);
				}
				else
				{
					CollectedWeapon[CurrentWeaponSlot]->DetachWeapon(this, NearestWeapon->GetTransform(), CurrentWeaponSlot);
					NearestWeapon->AttachWeapon(this);
				}
			}
		}
	}
	
	ClampHorizontalVelocity();

	if(SlideHot)
	{
		if(SlideCoolDown > 0.0f)
			SlideCoolDown -= DeltaSeconds;
		else
		{
			SlideCoolDown = SlideInterval;
			SlideHot = false;
		}
	}

	if(StaminaRegenHot)
	{
		if(StaminaRegenCoolDown > 0.0f)
			StaminaRegenCoolDown -= DeltaSeconds;
		else
		{
			StaminaRegenCoolDown = StaminaRegenInterval;
			StaminaRegenHot = false;
		}
	}
	
	if(ForwardAxis <= 0 && StateMachine->GetCurrentState() == EMovementState::Sprinting)
	{
		StateMachine->CheckStateTransition(EMovementState::Walking);
	}
	
	if(StateMachine->GetCurrentState() == EMovementState::Crouching && !ButtonPressed["Crouch"])
	{
		StateMachine->CheckStateTransition(EMovementState::Walking);
	}
	
	if(StateMachine->GetCurrentState() == EMovementState::Sprinting && !GetCharacterMovement()->IsFalling())
	{
		ConsumeStamina(SprintStaminaConsume * DeltaSeconds);
		if(CurrentStamina == 0)
		{
			StateMachine->CheckStateTransition(EMovementState::Walking);
		}
	}

	if(IsWallRunning)
	{
		ConsumeStamina(WallRunningStaminaConsume * DeltaSeconds);
		if(CurrentStamina == 0)
		{
			EndWallRunning(EWallRunningEndReason::FallOffWall);
		}
	}

	RegenStamina(DeltaSeconds);
}

void AFPSurvivalCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	SoundManager->PlaySound(TEXT("Land"), GetActorLocation());
	
	CurrentJumpCount = 0;
	WallRunningHot = false;
}

void AFPSurvivalCharacter::Jump()
{
	if(StateMachine->GetCurrentState() == EMovementState::Crouching)
		return;
	
	if(VaultingComponent->CanVault)
	{
		VaultingComponent->Vault();
	}
	else
	{
		bool JumpFlag = false;
		if(CurrentJumpCount == 0)
		{
			Super::Jump();
			JumpFlag = true;
		}
		else if (CurrentJumpCount < MaxJumpCount && CurrentStamina > 0)
		{
			ConsumeStamina(DoubleJumpStaminaConsume);
			JumpFlag = true;
		}
		
		if(JumpFlag)
		{
			const FVector LaunchDir = FindLaunchDirection();
			LaunchCharacter(LaunchDir, false, true);
			CurrentJumpCount++;

			if(IsWallRunning)
			{
				EndWallRunning(EWallRunningEndReason::JumpOffWall);
			}
		}
	}
}

void AFPSurvivalCharacter::OnPlayerDisable()
{
	CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
	HudWidget->SetVisibility(ESlateVisibility::Hidden);
	PickUpWidget->SetVisibility(ESlateVisibility::Hidden);
	GameStateWidget->SetVisibility(ESlateVisibility::Hidden);
	
	UGameplayStatics::SetGamePaused(this, true);
	
	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
	DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

float AFPSurvivalCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	
	if(bIsDead)
		return 0;
	
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

		if(IsPlayerControlled())
		{
			UHitIndicator* HitIndicator = Cast<UHitIndicator>(CreateWidget(GetWorld(), HitIndicatorClass));
			HitIndicator->HitDirection = PointDamageEvent->ShotDirection;
			HitIndicator->Character = this;
			HitIndicator->AddToViewport();
		}
	}
	
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID)) 
	{
		const FRadialDamageEvent* RadialDamageEvent = static_cast<const FRadialDamageEvent*>(&DamageEvent);
	}

	CurrentHP -= Damage;
	
	AFPSurvivalPlayerState* CurrentPlayerState = Cast<AFPSurvivalPlayerState>(GetPlayerState());
	CurrentPlayerState->AddDamageTaken(Damage);
	
	//SoundManager->PlaySound(TEXT("PlayerHit"), GetActorLocation());

	if(CurrentHP <= 0)
	{
		CurrentHP = 0;
		bIsDead = true;
		OnPlayerDisable();
		OnDead.Broadcast();
	}
	
    return Damage;
}

void AFPSurvivalCharacter::SetItemHoming(AItemPickup* Item) const
{
	if(Item == nullptr)
		return;

	if(!Item->IsHoming)
		Item->SetHomingTarget(ItemPickupRange);
}


void AFPSurvivalCharacter::OnItemHomingRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AItemPickup* Item = Cast<AItemPickup>(OtherActor);
	if(Item == nullptr)
		return;

	SetItemHoming(Item);
}

void AFPSurvivalCharacter::OnItemPickupRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AItemPickup* Item = Cast<AItemPickup>(OtherActor);
	if(Item == nullptr)
		return;
	
	switch(Item->ItemType)
	{
	case EItemType::Ammo:
		{
			const AAmmoPickup* AmmoPickup = Cast<AAmmoPickup>(Item);

			if(AmmoMap.Contains(AmmoPickup->AmmoName))
			{
				AmmoMap[AmmoPickup->AmmoName] += AmmoPickup->Amount;
			}
			else
			{
				AmmoMap.Add(AmmoPickup->AmmoName, AmmoPickup->Amount);
			}
			SoundManager->PlaySound(TEXT("AmmoPickup"), GetActorLocation());
			break;
		}

	case EItemType::Health:
		{
			CurrentHP += Item->Amount;
			if(CurrentHP > MaxHP)
				CurrentHP = MaxHP;
			SoundManager->PlaySound(TEXT("HealthPickup"), GetActorLocation());
			break;
		}
		
	default:
		break;
	}
	Item->Deactivate();
}

bool AFPSurvivalCharacter::WalkToCrouchTransition()
{
	if(ButtonPressed["Crouch"])
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::CrouchToWalkTransition()
{
	if(!ButtonPressed["Crouch"] && CanStand())
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::WalkToSprintTransition()
{
	if(CurrentWeapon != nullptr)
	{
		if(CurrentWeapon->GetIsFiring())
		{
			return false;
		}
	}
	if(IsWeaponChanging)
		return false;
	
	if(CanSprint() && ButtonPressed["Sprint"])
	{
		return true;
	}

	return false;
}

bool AFPSurvivalCharacter::SprintToWalkTransition()
{
	if(IsWallRunning)
	{
		return false;
	}
	
	if(CurrentWeapon != nullptr)
	{
		if(CurrentWeapon->GetIsFiring() || IsReloading || IsWeaponChanging || IsInSight)
		{
			return true;
		}
	}
	
	if(!ButtonPressed["Sprint"] || CurrentStamina == 0 || ForwardAxis <= 0)
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::SprintToSlideTransition()
{
	if(!GetCharacterMovement()->IsFalling() && !SlideHot && ButtonPressed["Crouch"] && CurrentStamina > 0)
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::SlideToCrouchTransition()
{
	if(!CanStand())
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::SlideToWalkTransition()
{
	if(!CanSprint() && CanStand())
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::SlideToSprintTransition()
{
	if(CanSprint() && GetVelocity().Length() < SpeedMap[EMovementState::Sprinting])
	{
		return true;
	}
	return false;
}

void AFPSurvivalCharacter::SprintInit()
{
	if(SpeedMap[EMovementState::Sprinting] != NULL)
		GetCharacterMovement()->MaxWalkSpeed = SpeedMap[EMovementState::Sprinting];
	
	if(IsReloading)
	{
		IsReloading = false;
		UE_LOG(LogTemp, Log, TEXT("SprintInit: %s"), IsReloading ? TEXT("true") : TEXT("false"));
		Mesh1P->GetAnimInstance()->Montage_Stop(0.1f);

		if(CurrentWeapon->GetMesh()->HasValidAnimationInstance())
			CurrentWeapon->GetMesh()->GetAnimInstance()->Montage_Stop(0.1f);
	}
	
	if(IsInSight)
	{
		IsInSight = false;
	}
}

void AFPSurvivalCharacter::SprintEnd()
{
	if(ButtonPressed["Sight"])
		OnSightAction(true);
}

void AFPSurvivalCharacter::WalkInit()
{
	if(SpeedMap[EMovementState::Walking] != NULL)
		GetCharacterMovement()->MaxWalkSpeed = SpeedMap[EMovementState::Walking];
}

void AFPSurvivalCharacter::SlideInit()
{
	ConsumeStamina(SlideStaminaConsume);
	
	SoundManager->PlaySound(TEXT("Slide"), GetActorLocation());
	SmoothCrouchingTimeline->Play();
	SlideTimeline->Play();
	GetCharacterMovement()->Velocity = GetActorForwardVector() * GetCharacterMovement()->Velocity.Length() * SlidePower;
	GetCharacterMovement()->GroundFriction = SlideGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration;
}

void AFPSurvivalCharacter::SlideEnd()
{
	SmoothCrouchingTimeline->Reverse();
	SlideTimeline->Stop();
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
	SlideHot = true;
}

void AFPSurvivalCharacter::CrouchInit()
{
	if(SpeedMap[EMovementState::Crouching] != NULL)
		GetCharacterMovement()->MaxWalkSpeed = SpeedMap[EMovementState::Crouching];
	SmoothCrouchingTimeline->Play();
}

void AFPSurvivalCharacter::CrouchEnd()
{
	SmoothCrouchingTimeline->Reverse();
}

void AFPSurvivalCharacter::SetStateMachineTransition()
{
	FStateTransition WalkToCrouch;
	WalkToCrouch.BindDynamic(this, &AFPSurvivalCharacter::WalkToCrouchTransition);
	StateMachine->AddTransitionFunc(WalkToCrouch, EMovementState::Walking, EMovementState::Crouching);

	FStateTransition WalkToSprint;
	WalkToSprint.BindDynamic(this, &AFPSurvivalCharacter::WalkToSprintTransition);
	StateMachine->AddTransitionFunc(WalkToSprint, EMovementState::Walking, EMovementState::Sprinting);

	FStateTransition CrouchToWalk;
	CrouchToWalk.BindDynamic(this, &AFPSurvivalCharacter::CrouchToWalkTransition);
	StateMachine->AddTransitionFunc(CrouchToWalk, EMovementState::Crouching, EMovementState::Walking);

	FStateTransition SprintToSlide;
	SprintToSlide.BindDynamic(this, &AFPSurvivalCharacter::SprintToSlideTransition);
	StateMachine->AddTransitionFunc(SprintToSlide, EMovementState::Sprinting, EMovementState::Sliding);

	FStateTransition SprintToWalk;
	SprintToWalk.BindDynamic(this, &AFPSurvivalCharacter::SprintToWalkTransition);
	StateMachine->AddTransitionFunc(SprintToWalk, EMovementState::Sprinting, EMovementState::Walking);

	FStateTransition SlideToCrouch;
	SlideToCrouch.BindDynamic(this, &AFPSurvivalCharacter::SlideToCrouchTransition);
	StateMachine->AddTransitionFunc(SlideToCrouch, EMovementState::Sliding, EMovementState::Crouching);

	FStateTransition SlideToWalk;
	SlideToWalk.BindDynamic(this, &AFPSurvivalCharacter::SlideToWalkTransition);
	StateMachine->AddTransitionFunc(SlideToWalk, EMovementState::Sliding, EMovementState::Walking);

	FStateTransition SlideToSprint;
	SlideToSprint.BindDynamic(this, &AFPSurvivalCharacter::SlideToSprintTransition);
	StateMachine->AddTransitionFunc(SlideToSprint, EMovementState::Sliding, EMovementState::Sprinting);

	FStateInit WalkInit;
	WalkInit.BindDynamic(this, &AFPSurvivalCharacter::WalkInit);
	StateMachine->AddInitFunc(WalkInit, EMovementState::Walking);

	FStateInit SprintInit;
	SprintInit.BindDynamic(this, &AFPSurvivalCharacter::SprintInit);
	StateMachine->AddInitFunc(SprintInit, EMovementState::Sprinting);

	FStateInit CrouchInit;
	CrouchInit.BindDynamic(this, &AFPSurvivalCharacter::CrouchInit);
	StateMachine->AddInitFunc(CrouchInit, EMovementState::Crouching);

	FStateInit SlideInit;
	SlideInit.BindDynamic(this, &AFPSurvivalCharacter::SlideInit);
	StateMachine->AddInitFunc(SlideInit, EMovementState::Sliding);
	
	FStateEnd WalkEnd;
	WalkEnd.BindDynamic(this, &AFPSurvivalCharacter::WalkEnd);
	StateMachine->AddEndFunc(WalkEnd, EMovementState::Walking);

	FStateEnd SprintEnd;
	SprintEnd.BindDynamic(this, &AFPSurvivalCharacter::SprintEnd);
	StateMachine->AddEndFunc(SprintEnd, EMovementState::Sprinting);

	FStateEnd CrouchEnd;
	CrouchEnd.BindDynamic(this, &AFPSurvivalCharacter::CrouchEnd);
	StateMachine->AddEndFunc(CrouchEnd, EMovementState::Crouching);

	FStateEnd SlideEnd;
	SlideEnd.BindDynamic(this, &AFPSurvivalCharacter::SlideEnd);
	StateMachine->AddEndFunc(SlideEnd, EMovementState::Sliding);
}

void AFPSurvivalCharacter::ConsumeStamina(float StaminaConsume)
{
	StaminaRegenHot = true;
	StaminaRegenCoolDown = StaminaRegenInterval;
	CurrentStamina -= StaminaConsume;
	if(CurrentStamina < 0)
		CurrentStamina = 0;
}

void AFPSurvivalCharacter::RegenStamina(float DeltaSeconds)
{
	if(!StaminaRegenHot && CurrentStamina < MaxStamina)
	{
		CurrentStamina += StaminaRegenValue * DeltaSeconds;
		if(CurrentStamina > MaxStamina)
		{
			CurrentStamina = MaxStamina;
		}
	}
}


//////////////////////////////////////////////////////////////////////////// Input

void AFPSurvivalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	DECLARE_DELEGATE_OneParam(FActionKeyDelegate, bool)
	DECLARE_DELEGATE_OneParam(FWeaponChangeDelegate, int)
	
	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction<FActionKeyDelegate>("PrimaryAction", IE_Pressed, this, &AFPSurvivalCharacter::OnPrimaryAction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("PrimaryAction", IE_Released, this, &AFPSurvivalCharacter::OnPrimaryAction, false);
	
	// Bind Run event
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Sprint", IE_Pressed, this, &AFPSurvivalCharacter::OnSprintAction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Sprint", IE_Released, this, &AFPSurvivalCharacter::OnSprintAction, false);

	PlayerInputComponent->BindAction<FActionKeyDelegate>("Crouch", IE_Pressed, this, &AFPSurvivalCharacter::OnCrouchAction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Crouch", IE_Released, this, &AFPSurvivalCharacter::OnCrouchAction, false);
	
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Reload", IE_Pressed, this, &AFPSurvivalCharacter::OnReloadAction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Reload", IE_Released, this, &AFPSurvivalCharacter::OnReloadAction, false);

	PlayerInputComponent->BindAction<FActionKeyDelegate>("Sight", IE_Pressed, this, &AFPSurvivalCharacter::OnSightAction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Sight", IE_Released, this, &AFPSurvivalCharacter::OnSightAction, false);

	PlayerInputComponent->BindAction<FActionKeyDelegate>("Interaction", IE_Pressed, this, &AFPSurvivalCharacter::OnInteraction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Interaction", IE_Released, this, &AFPSurvivalCharacter::OnInteraction, false);
	
	PlayerInputComponent->BindAction<FActionKeyDelegate>("DamageTest", IE_Pressed, this, &AFPSurvivalCharacter::OnDamageTest, true);
	
	PlayerInputComponent->BindAction<FWeaponChangeDelegate>("PrimaryWeapon", IE_Pressed, this, &AFPSurvivalCharacter::OnWeaponChange, 0);
	PlayerInputComponent->BindAction<FWeaponChangeDelegate>("SecondaryWeapon", IE_Pressed, this, &AFPSurvivalCharacter::OnWeaponChange, 1);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AFPSurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AFPSurvivalCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AFPSurvivalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AFPSurvivalCharacter::LookUpAtRate);
}

void AFPSurvivalCharacter::BeginWallRunning()
{
	CurrentJumpCount = 0;
	IsWallRunning = true;
	GetCharacterMovement()->AirControl = 1.0f;
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 0, 1));
	WallRunningTimeline->Play();
	CameraTiltTimeline->Play();
}

void AFPSurvivalCharacter::EndWallRunning(EWallRunningEndReason EndReason)
{
	IsWallRunning = false;
	if(EndReason == EWallRunningEndReason::FallOffWall)
	{
		WallRunningHot = true;
		CurrentJumpCount = 1;
	}
	else if(EndReason == EWallRunningEndReason::JumpOffWall)
	{
		CurrentJumpCount = MaxJumpCount - 1;
	}
	
	GetCharacterMovement()->AirControl = 0.05f;
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 0, 0));
	WallRunningTimeline->Stop();
	
	CameraTiltTimeline->Reverse();

	StateMachine->CheckStateTransition(EMovementState::Walking);
}

FWallRunningInfo AFPSurvivalCharacter::FindWallRunningDirectionAndSide(FVector WallNormal) const
{
	FVector2d RightVector2D(GetActorRightVector());
	FVector2d WallNormal2D(WallNormal);
	
	FWallRunningInfo ResultInfo;
	if(FVector2d::DotProduct(WallNormal2D, RightVector2D) > 0)
	{
		ResultInfo.Side = EWallRunningSide::Left;
		ResultInfo.Direction = WallNormal.Cross(FVector(0, 0, -1));
	}
	else
	{
		ResultInfo.Side = EWallRunningSide::Right;
		ResultInfo.Direction = WallNormal.Cross(FVector(0, 0, 1));
	}

	return ResultInfo;
}

void AFPSurvivalCharacter::RecoilTimelineReturn(float Value)
{
	float ResultPitch = IsInSight ? CurrentWeapon->RecoilPitchADS : CurrentWeapon->RecoilPitch;

	ResultPitch = FMath::Lerp(0, ResultPitch, Value);

	if(!RecoilTimeline->IsReversing())
	{
		AddControllerPitchInput(ResultPitch);

		const float RecoilYaw = CurrentWeapon->RecoilYaw;
		const float ResultYaw = FMath::RandRange(-RecoilYaw, RecoilYaw);
		AddControllerYawInput(ResultYaw);
	}
	else
	{
		AddControllerPitchInput(-ResultPitch);
	}	
}

void AFPSurvivalCharacter::ReverseRecoil()
{
	if(CurrentWeapon->RecoilOn)
		RecoilTimeline->ReverseFromEnd();
}


void AFPSurvivalCharacter::SmoothCrouchTimelineReturn(float Value)
{
	const auto CrouchedHeight = GetCharacterMovement()->GetCrouchedHalfHeight();
	GetCapsuleComponent()->SetCapsuleHalfHeight(FMath::Lerp(CrouchedHeight, StandingCapsuleHalfHeight, Value));

	GetMesh()->SetRelativeLocation(FVector(MeshDefaultRelativeLocation.X, MeshDefaultRelativeLocation.Y, MeshDefaultRelativeLocation.Z + (1 - Value) * TPMeshCrouchingZOffset));
	
	const auto RelativeLocation = FirstPersonCameraComponent->GetRelativeLocation();
	FirstPersonCameraComponent->SetRelativeLocation(FVector(RelativeLocation.X, RelativeLocation.Y, (FMath::Lerp(CrouchedEyeHeight, StandingCameraZOffset, Value))));
}

void AFPSurvivalCharacter::SlideTimelineReturn()
{
	const auto SlideDirection = CalculateFloorInfluence(GetCharacterMovement()->CurrentFloor.HitResult.Normal);

	GetCharacterMovement()->AddForce(SlideDirection);

	if(GetVelocity().Length() > SpeedMap[EMovementState::Sprinting] && GetCharacterMovement()->IsFalling())
	{
		auto Velocity = GetVelocity();
		Velocity.Normalize();
		
		GetCharacterMovement()->Velocity = Velocity * SpeedMap[EMovementState::Sprinting];
	}
	
	if((GetVelocity().Length() <= SpeedMap[EMovementState::Crouching] || GetCharacterMovement()->IsFalling()) && StateMachine->GetCurrentState() == EMovementState::Sliding)
	{
		bool TransitionCheck = StateMachine->CheckStateTransition(EMovementState::Sprinting);
		
		if(!TransitionCheck)
		{
			TransitionCheck = StateMachine->CheckStateTransition(EMovementState::Walking);
		}
		if(!TransitionCheck)
		{
			StateMachine->CheckStateTransition(EMovementState::Crouching);
		}
	}
}

void AFPSurvivalCharacter::ActionCheck()
{
	if(ButtonPressed["Sprint"])
	{
		Mesh1P->GetAnimInstance()->Montage_Stop(0.1f);
		OnSprintAction(true);

	}
	else if(ButtonPressed["Sight"])
	{
		OnSightAction(true);
	}
}
void AFPSurvivalCharacter::CameraTiltReturn(float Value)
{
	const auto ControlRotation = GetController()->GetControlRotation();
	if(StateMachine->GetCurrentState() == EMovementState::Sliding)
	{
		GetController()->SetControlRotation(FRotator(ControlRotation.Pitch, ControlRotation.Yaw, Value));
	}
	else
	{
		const float SideValue = WallRunningSide == EWallRunningSide::Left ? -1 : 1;
		
		GetController()->SetControlRotation(FRotator(ControlRotation.Pitch, ControlRotation.Yaw, Value * SideValue));
	}
}

void AFPSurvivalCharacter::OnPrimaryAction(const bool Pressed)
{
	if(CurrentWeapon == nullptr || IsWeaponChanging || IsReloading)
	{
		return;
	}
	
    if(Pressed)
    {
		if(CurrentWeapon->GetIsFiring())
			return;
    	
        OnFire[CurrentWeaponSlot].ExecuteIfBound(this);
    	
        if(StateMachine->GetCurrentState() == EMovementState::Sprinting)
        {
        	StateMachine->CheckStateTransition(EMovementState::Walking);
        }
    }
    else if(CurrentWeapon->CurrentAmmo > 0 && CurrentWeapon->GetIsFiring() && CurrentWeapon->FireMode == EFireMode::FullAuto)
    {
    	FullAutoEndFlag = true;
    }
}

void AFPSurvivalCharacter::OnReloadAction(const bool Pressed)
{
	if(CurrentWeapon == nullptr || IsWeaponChanging || !Pressed || IsReloading || AmmoMap[CurrentWeapon->WeaponName] <= 0)
	{
		return;
	}
	
	IsReloading = OnReload[CurrentWeaponSlot].Execute(Mesh1P->GetAnimInstance());
	
	if(IsReloading)
		IsInSight = false;
	
	if(StateMachine->GetCurrentState() == EMovementState::Sprinting)
	{
		StateMachine->CheckStateTransition(EMovementState::Walking);
	}
	UE_LOG(LogTemp, Log, TEXT("OnReloadAction: %s"), IsReloading ? TEXT("true") : TEXT("false") );
}

bool AFPSurvivalCharacter::CanStand()
{
	if(ButtonPressed["Crouch"])
	{
		return false;
	}
	const auto Location = GetActorLocation();

	const FVector Feet = FVector(Location.X, Location.Y, Location.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		
	const FVector Head = FVector(Feet.X, Feet.Y, Feet.Z + StandingCapsuleHalfHeight * 2);
	
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Feet, Head, ECC_Visibility, CollisionQueryParams);

	return !HitResult.bBlockingHit;
}

bool AFPSurvivalCharacter::CanSprint()
{
	if(ButtonPressed["Sprint"])
	{
		return !GetCharacterMovement()->IsFalling() && CanStand() && ForwardAxis > 0;
	}
	return false;
}

void AFPSurvivalCharacter::OnSightAction(bool Pressed)
{
	ButtonPressed["Sight"] = Pressed;
	if(CurrentWeapon == nullptr || IsReloading)
	{
		return;
	}

	SoundManager->PlaySound(TEXT("ADS"), GetActorLocation());
	if(Pressed)
	{
		IsInSight = true;
		
		UWidget* Bottom = CrosshairWidget->GetWidgetFromName("bottom");
		Bottom->SetRenderOpacity(0.0f);
		UWidget* Top = CrosshairWidget->GetWidgetFromName("Top");
		Top->SetRenderOpacity(0.0f);
		UWidget* Left = CrosshairWidget->GetWidgetFromName("Left");
		Left->SetRenderOpacity(0.0f);
		UWidget* Right = CrosshairWidget->GetWidgetFromName("Right");
		Right->SetRenderOpacity(0.0f);
		
		if(CurrentWeapon->DotCrosshairInSight)
		{
			UWidget* Dot = HudWidget->GetWidgetFromName("DotCrosshair");
			Dot->SetRenderOpacity(1.0f);
		}
		else
		{
			UWidget* Dot = HudWidget->GetWidgetFromName("DotCrosshair");
			Dot->SetRenderOpacity(0.0f);
		}
		
		if(StateMachine->GetCurrentState() == EMovementState::Sprinting)
		{
			StateMachine->CheckStateTransition(EMovementState::Walking);
		}
	}
	else
	{
		UWidget* Bottom = CrosshairWidget->GetWidgetFromName("bottom");
		Bottom->SetRenderOpacity(1.0f);
		UWidget* Top = CrosshairWidget->GetWidgetFromName("Top");
		Top->SetRenderOpacity(1.0f);
		UWidget* Left = CrosshairWidget->GetWidgetFromName("Left");
		Left->SetRenderOpacity(1.0f);
		UWidget* Right = CrosshairWidget->GetWidgetFromName("Right");
		Right->SetRenderOpacity(1.0f);
		
		if(CurrentWeapon->DotCrosshairInSight)
		{
			UWidget* Dot = HudWidget->GetWidgetFromName("DotCrosshair");
			Dot->SetRenderOpacity(0.0f);
		}
		
		IsInSight = false;
		ActionCheck();
	}
}

void AFPSurvivalCharacter::OnInteraction(bool Pressed)
{
	if(Pressed)
	{
		ButtonPressed["Interaction"] = true;
	}
	else
	{
		ButtonPressed["Interaction"] = false;
	}
}

void AFPSurvivalCharacter::OnDamageTest(bool Pressed)
{
	if(Pressed)
	{
		const FHitResult HitInfo;
		FVector LocationFired = GetActorLocation();
		LocationFired.Y -= 50;
		
		FVector HitDirection = GetActorLocation() - LocationFired;
		HitDirection.Normalize();
		
		UGameplayStatics::ApplyPointDamage(this, 50, HitDirection, HitInfo, GetInstigatorController(), this, nullptr);
	}
}

void AFPSurvivalCharacter::OnSprintAction(const bool Pressed)
{
	ButtonPressed["Sprint"] = Pressed;
	if(Pressed)
	{
		StateMachine->CheckStateTransition(EMovementState::Sprinting);
	}
	else
	{
		StateMachine->CheckStateTransition(EMovementState::Walking);
	}
}

void AFPSurvivalCharacter::OnCrouchAction(const bool Pressed)
{
	ButtonPressed["Crouch"] = Pressed;
	if(Pressed)
	{
		if(StateMachine->GetCurrentState() != EMovementState::Sprinting)
			StateMachine->CheckStateTransition(EMovementState::Crouching);
		else
			StateMachine->CheckStateTransition(EMovementState::Sliding);

	}
	else
	{
		if(StateMachine->GetCurrentState() == EMovementState::Crouching)
			StateMachine->CheckStateTransition(EMovementState::Walking);
	}
}

void AFPSurvivalCharacter::OnWeaponChange(int WeaponNum)
{
	if(CollectedWeapon.Num() > WeaponNum && !IsWeaponChanging)
	{
		bool ChangeFlag = false;
		if(CurrentWeapon != nullptr)
		{
			if(CurrentWeapon != CollectedWeapon[WeaponNum] && !CurrentWeapon->GetIsFiring())
			{
				IsInSight = false;
				ChangeFlag = true;
				
				if(IsReloading)
				{
					IsReloading = false;
				}
			
				Mesh1P->GetAnimInstance()->Montage_Stop(0.0f);
				
				if(CurrentWeapon->GetMesh()->HasValidAnimationInstance())
					CurrentWeapon->GetMesh()->GetAnimInstance()->Montage_Stop(0.0f);
				
				Mesh1P->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponPullDownMontage);
				ChangingWeaponSlot = WeaponNum;
			}
		}
		else
		{
			ChangeFlag = true;
			CurrentWeaponSlot = WeaponNum;
			CurrentWeapon = CollectedWeapon[WeaponNum];

			if(HudWidget != nullptr)
				HudWidget->CurrentWeaponImage = CurrentWeapon->WeaponImage;
			
			CurrentWeapon->SetActorHiddenInGame(false); 
			CurrentWeapon->SetActorEnableCollision(true); 
			CurrentWeapon->SetActorTickEnabled(true);
			
			Mesh1P->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponPullUpMontage);
		}

		if(ChangeFlag)
		{
			IsWeaponChanging = true;
			if(StateMachine->GetCurrentState() == EMovementState::Sprinting)
			{
				StateMachine->CheckStateTransition(EMovementState::Walking);
			}
		}
	}
}

void AFPSurvivalCharacter::OnWeaponChangeEnd()
{
	if(CurrentWeapon->GetIsAttached())
	{
		CurrentWeapon->SetActorHiddenInGame(true); 
		CurrentWeapon->SetActorEnableCollision(false); 
		CurrentWeapon->SetActorTickEnabled(false);
	}

	CollectedWeapon[ChangingWeaponSlot]->SetActorHiddenInGame(false); 
	CollectedWeapon[ChangingWeaponSlot]->SetActorEnableCollision(true); 
	CollectedWeapon[ChangingWeaponSlot]->SetActorTickEnabled(true);

	CurrentWeapon = CollectedWeapon[ChangingWeaponSlot];

	if(HudWidget != nullptr)
		HudWidget->CurrentWeaponImage = CurrentWeapon->WeaponImage;
	
	CurrentWeapon->SoundManager->PlaySound(TEXT("Equip"), GetActorLocation());
	
	CurrentWeaponSlot = ChangingWeaponSlot;
	ChangingWeaponSlot = -1;
	Mesh1P->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponPullUpMontage);
}

void AFPSurvivalCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == CurrentWeapon->WeaponPullUpMontage && !bInterrupted)
	{
		IsWeaponChanging = false;
		
		ActionCheck();
	}

	/*if (Montage == CurrentWeapon->ArmFireMontage && bInterrupted)
	{
		if (CurrentWeapon->GetIsFiring() && CurrentWeapon->FireMode == EFireMode::FullAuto)
		{
			FullAutoEndFlag = true;
			FullAutoFireEnded();
		}
	}*/
}

void AFPSurvivalCharacter::FullAutoFireEnded()
{
	if(FullAutoEndFlag || CurrentWeapon->CurrentAmmo <= 0)
	{
		FullAutoEndFlag = false;
		    		
		ReverseRecoil();
					
		OnFireEnd[CurrentWeaponSlot].ExecuteIfBound();
	}
	else
	{
		OnFire[CurrentWeaponSlot].ExecuteIfBound(this);
	}
}

void AFPSurvivalCharacter::OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(IsWallRunning || StateMachine->GetCurrentState() != EMovementState::Sprinting)
	{
		return;
	}
	
	if(CanSurfaceBeWallRan(Hit.ImpactNormal) &&	GetCharacterMovement()->IsFalling() && !WallRunningHot)
	{
		const auto [Side, Direction] = FindWallRunningDirectionAndSide(Hit.ImpactNormal);
		WallRunningDirection = Direction;
		WallRunningSide = Side;
		
		if(IsWallRunningKeysDown())
		{
			BeginWallRunning();
		}
	}
}

void AFPSurvivalCharacter::DamageToOtherActor(bool Headshot, bool Dead, float Damage)
{
	if(CrosshairWidget == nullptr)
	{
		return;
	}
	
	if(Headshot)
	{
		SoundManager->PlaySound(TEXT("EnemyHeadshot"), GetActorLocation());
		CrosshairWidget->HitIndicatorColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		SoundManager->PlaySound(TEXT("EnemyHit"), GetActorLocation());
		CrosshairWidget->HitIndicatorColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	AFPSurvivalPlayerState* CurrentPlayerState = Cast<AFPSurvivalPlayerState>(GetPlayerState());

	if(Dead)
		CurrentPlayerState->AddKillScore();

	CurrentPlayerState->AddDamageDealt(Damage);
}

FVector AFPSurvivalCharacter::CalculateFloorInfluence(FVector FloorNormal)
{
	const auto UpVector = GetActorUpVector();
	if(UpVector == FloorNormal)
	{
		return FVector(0, 0, 0);
	}

	const auto FloorInfluence = FMath::Clamp(1 - FVector::DotProduct(FloorNormal, UpVector), 0, 1) * SlideFloorInfluence;
	
	FVector NormalizedResult = FloorNormal.Cross(FloorNormal.Cross(UpVector));
	NormalizedResult.Normalize();
	
	return NormalizedResult * FloorInfluence;
}

void AFPSurvivalCharacter::MoveForward(float Value)
{
	ForwardAxis = Value;
	// if(Value <= 0 && StateMachine->GetCurrentState() == EMovementState::Sprinting)
	// {
	// 	StateMachine->CheckStateTransition(EMovementState::Walking);
	// }
	
	if (Value != 0.0f && StateMachine->GetCurrentState() != EMovementState::Sliding)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSurvivalCharacter::MoveRight(float Value)
{
	RightAxis = Value;
	if (Value != 0.0f && StateMachine->GetCurrentState() != EMovementState::Sliding)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSurvivalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFPSurvivalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

bool AFPSurvivalCharacter::CanSurfaceBeWallRan(FVector SurfaceNormal) const
{
	if(SurfaceNormal.Z < -0.05f)
	{
		return false;
	}

	FVector SlopeNormal = FVector(SurfaceNormal.X, SurfaceNormal.Y, 0.f);
	SlopeNormal.Normalize();
	
	const float SlopeAngle = FMath::Acos(FVector::DotProduct(SlopeNormal, SurfaceNormal));

	return SlopeAngle < GetCharacterMovement()->GetWalkableFloorAngle();
}

FVector AFPSurvivalCharacter::FindLaunchDirection() const
{
	FVector LaunchDirection;
	if(IsWallRunning)
	{
		if(WallRunningSide == EWallRunningSide::Left)
			LaunchDirection = WallRunningDirection.Cross(FVector(0, 0, 1));
		else
			LaunchDirection = WallRunningDirection.Cross(FVector(0, 0, -1));
	}
	else
	{
		if(GetCharacterMovement()->IsFalling())
		{
			LaunchDirection = GetActorRightVector() * RightAxis + GetActorForwardVector() * ForwardAxis;
		}
	}
	
	return (LaunchDirection + FVector(0, 0, 1)) * GetCharacterMovement()->JumpZVelocity;
}

bool AFPSurvivalCharacter::IsWallRunningKeysDown() const
{
	return ForwardAxis > 0.1;
}

void AFPSurvivalCharacter::ClampHorizontalVelocity() const
{
	if(GetCharacterMovement()->IsFalling() && StateMachine->GetCurrentState() != EMovementState::Sliding)
	{
		const auto BaseVelocity = GetHorizontalVelocity().Length() / GetCharacterMovement()->GetMaxSpeed();
		if(BaseVelocity > 1.0f)
		{
			const auto ModifiedVelocity = GetHorizontalVelocity() / BaseVelocity;
			SetHorizontalVelocity(ModifiedVelocity.X, ModifiedVelocity.Y);
		}
	}
}

void AFPSurvivalCharacter::UpdateWallRunning()
{
	if(!IsWallRunningKeysDown())
	{
		EndWallRunning(EWallRunningEndReason::FallOffWall);
	}
	
	FHitResult HitResult;

	FVector LocationStart = GetActorLocation();
	FVector LocationEnd;
	
	if(WallRunningSide == EWallRunningSide::Left)
	{
		LocationEnd = FVector(0, 0, -1);
	}
	else
	{
		LocationEnd = FVector(0, 0, 1);
	}

	LocationEnd = WallRunningDirection.Cross(LocationEnd) * 80 + LocationStart;
	
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	bool TraceCheck = GetWorld()->LineTraceSingleByChannel(HitResult, LocationStart, LocationEnd, ECC_Visibility, CollisionQueryParams);
	
	if(!TraceCheck)
	{
		EndWallRunning(EWallRunningEndReason::FallOffWall);
	}
	else
	{
		const auto [Side, Direction] = FindWallRunningDirectionAndSide(HitResult.ImpactNormal);
		if(WallRunningSide != Side)
		{
			EndWallRunning(EWallRunningEndReason::FallOffWall);
		}
		else
		{
			WallRunningDirection = Direction;
			FVector Velocity = WallRunningDirection * SpeedMap[EMovementState::Sprinting];
			GetCharacterMovement()->Velocity = FVector(-Velocity.X, -Velocity.Y, 0.f);
		}
	}
}

void AFPSurvivalCharacter::SetHorizontalVelocity(float VelocityX, float VelocityY) const
{
	GetCharacterMovement()->Velocity.X = VelocityX;
	GetCharacterMovement()->Velocity.Y = VelocityY;
}

int AFPSurvivalCharacter::GetCurrentWeaponID() const
{
	return CurrentWeapon->WeaponID;
}

bool AFPSurvivalCharacter::IsSprinting() const
{
	if(StateMachine->GetCurrentState() == EMovementState::Sprinting)
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::IsCrouching() const
{
	if(StateMachine->GetCurrentState() == EMovementState::Crouching)
	{
		return true;
	}
	return false;
}

void AFPSurvivalCharacter::OnWaveStart() const
{
	UWidget* WaveReadyTextBox = GameStateWidget->GetWidgetFromName(TEXT("WaveReadyTextBox"));
	WaveReadyTextBox->SetVisibility(ESlateVisibility::Hidden);

	UWidget* CurrentWaveTextBox = GameStateWidget->GetWidgetFromName(TEXT("CurrentWaveTextBox"));
	CurrentWaveTextBox->SetVisibility(ESlateVisibility::Visible);

	UWidget* WaveRemainTimeTextBox = GameStateWidget->GetWidgetFromName(TEXT("WaveRemainTimeTextBox"));
	WaveRemainTimeTextBox->SetVisibility(ESlateVisibility::Visible);

	UWidget* ZombieLeftTextBox = GameStateWidget->GetWidgetFromName(TEXT("ZombieLeftTextBox"));
	ZombieLeftTextBox->SetVisibility(ESlateVisibility::Visible);
	
	GameStateWidget->CurrentWaveString = FString::Printf(TEXT("Current Wave : %d/%d"), GameStateWidget->CurrentWave, GameStateWidget->MaxWave);
}

void AFPSurvivalCharacter::OnWaveReady(int CurrentWave)
{
	UWidget* WaveReadyTextBox = GameStateWidget->GetWidgetFromName(TEXT("WaveReadyTextBox"));
	WaveReadyTextBox->SetVisibility(ESlateVisibility::Visible);
	
	UWidget* CurrentWaveTextBox = GameStateWidget->GetWidgetFromName(TEXT("CurrentWaveTextBox"));
	CurrentWaveTextBox->SetVisibility(ESlateVisibility::Hidden);
	
	UWidget* WaveRemainTimeTextBox = GameStateWidget->GetWidgetFromName(TEXT("WaveRemainTimeTextBox"));
	WaveRemainTimeTextBox->SetVisibility(ESlateVisibility::Hidden);

	UWidget* ZombieLeftTextBox = GameStateWidget->GetWidgetFromName(TEXT("ZombieLeftTextBox"));
	ZombieLeftTextBox->SetVisibility(ESlateVisibility::Hidden);
    	
	GameStateWidget->CurrentWave = CurrentWave;
}

void AFPSurvivalCharacter::SetMaxWaveInfo(int MaxWaveInfo)
{
	if(GameStateWidget != nullptr)
	{
		GameStateWidget->MaxWave = MaxWaveInfo;
	}
}

void AFPSurvivalCharacter::SetWaveReadyRemainTime(FTimespan RemainTime)
{
	if(GameStateWidget != nullptr)
	{
		GameStateWidget->ReadyRemainTime = RemainTime;
		GameStateWidget->WaveReadyString = FString::Printf(TEXT("Wave %d\n"), GameStateWidget->CurrentWave);

		FString TimeString = FString::FromInt(GameStateWidget->ReadyRemainTime.GetTotalSeconds());
		FString MilliSecondString =  FString(GameStateWidget->ReadyRemainTime.ToString(TEXT(".%f")));
		MilliSecondString.RemoveAt(0);
		
		GameStateWidget->WaveReadyString += TimeString + MilliSecondString;
	}
}

void AFPSurvivalCharacter::SetWaveProgressRemainTime(FTimespan RemainTime)
{
	if(GameStateWidget != nullptr)
	{
		GameStateWidget->WaveProgressRemainTime = RemainTime;
		
		FString TimeString = FString::FromInt(GameStateWidget->WaveProgressRemainTime.GetTotalSeconds());
		FString MilliSecondString = FString(GameStateWidget->WaveProgressRemainTime.ToString(TEXT(".%f")));
		MilliSecondString.RemoveAt(0);
		
		GameStateWidget->WaveRemainTimeString = TimeString + MilliSecondString;
	}
}

void AFPSurvivalCharacter::SetZombieCounter(int ZombieCounter)
{
	if(GameStateWidget != nullptr)
	{
		GameStateWidget->ZombieLeftString = FString::Printf(TEXT("Zombie Left: %d"), ZombieCounter);
	}
}
