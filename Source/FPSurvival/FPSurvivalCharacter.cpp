// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSurvivalCharacter.h"
#include "FPSurvivalProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "VaultingComponent.h"
#include "WallRunningComponent.h"
#include "Components/WidgetComponent.h"
#include "CrossHairWidget.h"
#include "PickUpWidget.h"
#include "WeaponBase.h"
#include "Kismet/KismetMathLibrary.h"


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
	
	StateMachine = CreateDefaultSubobject<UMovementStateMachine>(TEXT("StateMachine"));
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(false);
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
	
	VaultingComponent = CreateDefaultSubobject<UVaultingComponent>(TEXT("VaultingObject"));
	WallRunningComponent = CreateDefaultSubobject<UWallRunningComponent>(TEXT("WallRunningObject"));
	
	StandingCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	StandingCameraZOffset = GetFirstPersonCameraComponent()->GetRelativeLocation().Z;

	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
	
	SlideCoolTime = SlideInterval;
	
	static ConstructorHelpers::FClassFinder<UUserWidget> UI_CrossHair(TEXT("/Game/FirstPerson/Widgets/WBCrosshair.WBCrosshair_C"));
	if(UI_CrossHair.Succeeded())
	{
		CrossHairWidget = CreateWidget<UCrossHairWidget>(GetWorld(), UI_CrossHair.Class, TEXT("CrossHair"));
		if(CrossHairWidget != nullptr)
			CrossHairWidget->AddToViewport();
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> UI_HUD(TEXT("/Game/FirstPerson/Widgets/WBHud.WBHud_C"));
	if(UI_HUD.Succeeded())
	{
		HudWidget = CreateWidget<UHudWidget>(GetWorld(), UI_HUD.Class, TEXT("Hud"));
		if(HudWidget != nullptr)
			HudWidget->AddToViewport();
	}
}

void AFPSurvivalCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

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
	
	SlideTimelineFunction.BindUFunction(this, FName("SlideTimelineReturn"));
	SlideTimeline->AddEvent(0, SlideTimelineFunction);
	SlideTimeline->SetTimelineLength(1.0);
	SlideTimeline->SetLooping(true);
	
	RecoilTimelineFunction.BindUFunction(this, FName("RecoilTimelineReturn"));
	if(RecoilCurveFloat)
	{
		RecoilTimeline->AddInterpFloat(RecoilCurveFloat, RecoilTimelineFunction);
		RecoilTimeline->SetTimelineLength(0.1f);
		RecoilTimeline->SetLooping(false);
	}
	
	Mesh1P->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AFPSurvivalCharacter::OnMontageEnd);
	
	PickUpWidget = Cast<UPickUpWidget>(HudWidget->GetWidgetFromName(TEXT("WBPickUp")));
}

void AFPSurvivalCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(SlideHot)
	{
		if(SlideCoolTime > 0.0f)
			SlideCoolTime -= DeltaSeconds;
		else
		{
			SlideCoolTime = SlideInterval;
			SlideHot = false;
		}
	}

	CrossHairWidget->Spread = FMath::GetMappedRangeValueClamped(FVector2D(0, 1000), FVector2D(5, 80), GetVelocity().Length());
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
	
	if(StateMachine->GetCurrentState() == EMovementState::Crouching && !ButtonPressed["Crouch"])
	{
		StateMachine->CheckStateTransition(EMovementState::Walking);
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

void AFPSurvivalCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	CurrentJumpCount = 0;
}

void AFPSurvivalCharacter::Jump()
{
	if(VaultingComponent->CanVault)
	{
		VaultingComponent->Vault();
	}
	else
	{
		if(CurrentJumpCount == 0)
		{
			Super::Jump();
			CurrentJumpCount++;
		}
		else if (CurrentJumpCount < MaxJumpCount)
		{
			const FVector LaunchDir = FVector(0, 0, GetCharacterMovement()->JumpZVelocity);
			LaunchCharacter(LaunchDir, false, true);
			CurrentJumpCount++;
		}
	}
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
	if(CurrentWeapon != nullptr)
	{
		if(CurrentWeapon->GetIsFiring() || IsReloading || IsWeaponChanging || IsInSight)
		{
			return true;
		}
	}
	
	if(!ButtonPressed["Sprint"])
	{
		return true;
	}
	return false;
}

bool AFPSurvivalCharacter::SprintToSlideTransition()
{
	if(!GetCharacterMovement()->IsFalling() && !SlideHot && ButtonPressed["Crouch"])
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
	SmoothCrouchingTimeline->Play();
	SlideTimeline->Play();
	CameraTiltTimeline->Play();
	GetCharacterMovement()->Velocity = GetActorForwardVector() * GetCharacterMovement()->Velocity.Length() * SlidePower;
	GetCharacterMovement()->GroundFriction = SlideGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration;
}

void AFPSurvivalCharacter::SlideEnd()
{
	SmoothCrouchingTimeline->Reverse();
	SlideTimeline->Stop();
	CameraTiltTimeline->Reverse();
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
	
	PlayerInputComponent->BindAction<FWeaponChangeDelegate>("PrimaryWeapon", IE_Pressed, this, &AFPSurvivalCharacter::OnWeaponChange, 0);
	PlayerInputComponent->BindAction<FWeaponChangeDelegate>("SecondaryWeapon", IE_Pressed, this, &AFPSurvivalCharacter::OnWeaponChange, 1);
	
	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

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


void AFPSurvivalCharacter::SmoothCrouchTimelineReturn(float Value)
{
	const auto CrouchedHeight = GetCharacterMovement()->GetCrouchedHalfHeight();
	GetCapsuleComponent()->SetCapsuleHalfHeight(FMath::Lerp(CrouchedHeight, StandingCapsuleHalfHeight, Value));

	const auto RelativeLocation = FirstPersonCameraComponent->GetRelativeLocation();
	FirstPersonCameraComponent->SetRelativeLocation(FVector(RelativeLocation.X, RelativeLocation.Y, (FMath::Lerp(CrouchedEyeHeight, StandingCameraZOffset, Value))));
}

void AFPSurvivalCharacter::SlideTimelineReturn()
{
	const auto SlideDirection = CalculateFloorInfluence(GetCharacterMovement()->CurrentFloor.HitResult.Normal);

	GetCharacterMovement()->AddForce(SlideDirection);

	if(GetVelocity().Length() > SpeedMap[EMovementState::Sprinting] && GetCharacterMovement()->IsFalling())
	{
		auto velocity = GetVelocity();
		velocity.Normalize();
		
		GetCharacterMovement()->Velocity = velocity * SpeedMap[EMovementState::Sprinting];
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
	if(ButtonPressed["Sight"])
	{
		OnSightAction(true);
	}
}
void AFPSurvivalCharacter::CameraTiltReturn(float Value)
{
	auto ControlRotation = GetController()->GetControlRotation();
	GetController()->SetControlRotation(FRotator(ControlRotation.Pitch, ControlRotation.Yaw, Value));
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
    else if(CurrentWeapon->CurrentAmmo > 0 && CurrentWeapon->GetIsFiring())
    {
    	FireEndFlag = true;
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

	FHitResult HitResult;

	GetWorld()->LineTraceSingleByChannel(HitResult, Feet, Head, ECC_Visibility);

	return !HitResult.bBlockingHit;
}

bool AFPSurvivalCharacter::CanSprint()
{
	if(ButtonPressed["Sprint"])
	{
		return !GetCharacterMovement()->IsFalling() && CanStand();
	}
	return false;
}

void AFPSurvivalCharacter::BeginSlide()
{
	SlideTimeline->Play();
	CameraTiltTimeline->Play();
	GetCharacterMovement()->Velocity = GetActorForwardVector() * GetCharacterMovement()->Velocity.Length() * SlidePower;
	GetCharacterMovement()->GroundFriction = SlideGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration;
}

void AFPSurvivalCharacter::EndSlide()
{
	SlideTimeline->Stop();
	CameraTiltTimeline->Reverse();
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
	SlideHot = true;
}

void AFPSurvivalCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction(true);
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFPSurvivalCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}


void AFPSurvivalCharacter::OnSightAction(bool Pressed)
{
	ButtonPressed["Sight"] = Pressed;
	if(CurrentWeapon == nullptr || IsReloading)
	{
		return;
	}
	
	if(Pressed)
	{
		IsInSight = true;
		if(StateMachine->GetCurrentState() == EMovementState::Sprinting)
		{
			StateMachine->CheckStateTransition(EMovementState::Walking);
		}
	}
	else
	{
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

	CurrentWeaponSlot = ChangingWeaponSlot;
	ChangingWeaponSlot = -1;
	Mesh1P->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponPullUpMontage);
}

void AFPSurvivalCharacter::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == CurrentWeapon->WeaponPullUpMontage && !bInterrupted)
	{
		HudWidget->CurrentWeaponImage = CurrentWeapon->WeaponImage;
		IsWeaponChanging = false;
		
		ActionCheck();
	}
	else if(Montage == CurrentWeapon->WeaponPullDownMontage && !bInterrupted)
	{
		OnWeaponChangeEnd();
	}
	else if(Montage == CurrentWeapon->ArmFireMontage || Montage == CurrentWeapon->ArmAimDownSightFireMontage)
	{
		if(CurrentWeapon->FireMode == EFireMode::FullAuto)
		{
			if(FireEndFlag || CurrentWeapon->CurrentAmmo <= 0)
			{
				FireEndFlag = false;
		    	
				if(CurrentWeapon->RecoilOn)
					RecoilTimeline->ReverseFromEnd();
				
				OnFireEnd[CurrentWeaponSlot].ExecuteIfBound();
				ActionCheck();
			}
			else
			{
				CurrentWeapon->Fire(this);
			}
		}
		else if(CurrentWeapon->FireMode == EFireMode::Single)
		{
			if(CurrentWeapon->RecoilOn)
				RecoilTimeline->ReverseFromEnd();
		}
	}
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
	NormalizedResult.Normalize(0.001);
	
	return NormalizedResult * FloorInfluence;
}

void AFPSurvivalCharacter::MoveForward(float Value)
{
	if (Value != 0.0f && StateMachine->GetCurrentState() != EMovementState::Sliding)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSurvivalCharacter::MoveRight(float Value)
{
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

bool AFPSurvivalCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFPSurvivalCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFPSurvivalCharacter::EndTouch);

		return true;
	}
	
	return false;
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
