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


AFPSurvivalCharacter::AFPSurvivalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	SprintMultiplier = 1.7f;
	CrouchMultiplier = 0.6f;

	SpeedMap.Reserve(3);

	SpeedMap.Add(EMovementState::Walking, GetCharacterMovement()->MaxWalkSpeed);
	SpeedMap.Add(EMovementState::Sprinting, GetCharacterMovement()->MaxWalkSpeed * SprintMultiplier);
	SpeedMap.Add(EMovementState::Crouching, GetCharacterMovement()->MaxWalkSpeed * CrouchMultiplier);
	SpeedMap.Add(EMovementState::Sliding, NULL);
	
	ButtonPressed.Reserve(2);

	ButtonPressed.Add("Sprint", false);
	ButtonPressed.Add("Crouch", false);
	
	CurrentJumpCount = 0;
	MaxJumpCount = 2;
	
	MovementState = EMovementState::Walking;
	PrevMovementState = EMovementState::Walking;

	SmoothCrouchingTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SmoothCrouchingTimeline"));
	SlideTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SlideTimeline"));
	CameraTiltTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TiltTimeline"));

	VaultingComponent = CreateDefaultSubobject<UVaultingComponent>(TEXT("VaultingObject"));
	
	StandingCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	StandingCameraZOffset = GetFirstPersonCameraComponent()->GetRelativeLocation().Z;

	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
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
}

void AFPSurvivalCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MovementState == EMovementState::Crouching && !ButtonPressed["Crouch"])
	{
		ResolveMovementState();
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
//////////////////////////////////////////////////////////////////////////// Input

void AFPSurvivalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	DECLARE_DELEGATE_OneParam(FActionKeyDelegate, bool)
	
	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AFPSurvivalCharacter::OnPrimaryAction);

	// Bind Run event
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Sprint", IE_Pressed, this, &AFPSurvivalCharacter::OnSprintAction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Sprint", IE_Released, this, &AFPSurvivalCharacter::OnSprintAction, false);

	PlayerInputComponent->BindAction<FActionKeyDelegate>("Crouch", IE_Pressed, this, &AFPSurvivalCharacter::OnCrouchAction, true);
	PlayerInputComponent->BindAction<FActionKeyDelegate>("Crouch", IE_Released, this, &AFPSurvivalCharacter::OnCrouchAction, false);
	
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

	if(GetVelocity().Length() > SpeedMap[EMovementState::Sprinting])
	{
		auto velocity = GetVelocity();
		velocity.Normalize();
		
		GetCharacterMovement()->Velocity = velocity * SpeedMap[EMovementState::Sprinting];
	}
	if(GetVelocity().Length() < SpeedMap[EMovementState::Crouching])
	{
		ResolveMovementState();
	}
}

void AFPSurvivalCharacter::CameraTiltReturn(float Value)
{
	auto ControlRotation = GetController()->GetControlRotation();
	GetController()->SetControlRotation(FRotator(ControlRotation.Pitch, ControlRotation.Yaw, Value));
}

void AFPSurvivalCharacter::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	OnUseItem.Broadcast();
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
	GetCharacterMovement()->Velocity = GetActorForwardVector() * GetCharacterMovement()->Velocity.Length();
	GetCharacterMovement()->GroundFriction = SlideGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration;
}

void AFPSurvivalCharacter::EndSlide()
{
	SlideTimeline->Stop();
	CameraTiltTimeline->Reverse();
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
}

void AFPSurvivalCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
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

void AFPSurvivalCharacter::OnSprintAction(const bool Pressed)
{
	if(Pressed)
	{
		ButtonPressed["Sprint"] = true;
		if(MovementState == EMovementState::Walking && CanSprint())
			SetMovementState(EMovementState::Sprinting);
	}
	else
	{
		ButtonPressed["Sprint"] = false;
		if(MovementState == EMovementState::Sprinting)
			ResolveMovementState();
	}
}

void AFPSurvivalCharacter::OnCrouchAction(const bool Pressed)
{
	if(Pressed)
	{
		ButtonPressed["Crouch"] = true;
		SmoothCrouchingTimeline->Play();

		if(MovementState == EMovementState::Walking)
		{
			SetMovementState(EMovementState::Crouching);
		}

		if(MovementState == EMovementState::Sprinting)
		{
			if (!GetCharacterMovement()->IsFalling())
			{
				SetMovementState(EMovementState::Sliding);
			}
			else
			{
				SetMovementState(EMovementState::Crouching);
			}
		}

		if (GetCharacterMovement()->IsFalling())
		{
			GetCharacterMovement()->Velocity = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, GroundSmashForce);
		}

	}
	else
	{
		ButtonPressed["Crouch"] = false;
		ResolveMovementState();
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

void AFPSurvivalCharacter::SetMovementState(const EMovementState NewMovementState)
{
	if(NewMovementState != MovementState)
	{
		PrevMovementState = MovementState;
		MovementState = NewMovementState;

		OnMovementStateChanged();
	}
}

void AFPSurvivalCharacter::ResolveMovementState()
{
	if(CanStand())
	{
		SmoothCrouchingTimeline->Reverse();
		if(CanSprint())
		{
			SetMovementState(EMovementState::Sprinting);
		}
		else
		{
			SetMovementState(EMovementState::Walking);
		}
	}
	else
	{
		SetMovementState(EMovementState::Crouching);
	}
}

void AFPSurvivalCharacter::OnMovementStateChanged()
{
	if(SpeedMap[MovementState] != NULL)
		GetCharacterMovement()->MaxWalkSpeed = SpeedMap[MovementState];

	if(MovementState == EMovementState::Sliding)
		BeginSlide();

	if(PrevMovementState == EMovementState::Sliding)
		EndSlide();
}

void AFPSurvivalCharacter::MoveForward(float Value)
{
	if (Value != 0.0f && MovementState != EMovementState::Sliding)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSurvivalCharacter::MoveRight(float Value)
{
	if (Value != 0.0f && MovementState != EMovementState::Sliding)
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
