// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieCharacter.h"

#include "FPSurvivalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AZombieCharacter::AZombieCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeleeAttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MeleeAttackSphere"));
	MeleeAttackSphere->SetupAttachment(GetMesh(), TEXT("RightHand"));
	MeleeAttackSphere->SetSphereRadius(15.0f);
}

// Called when the game starts or when spawned
void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AZombieCharacter::MontageEnded);
	MeleeAttackSphere->OnComponentBeginOverlap.AddDynamic(this, &AZombieCharacter::OnSphereBeginOverlap);
}

// Called every frame
void AZombieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AZombieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

bool AZombieCharacter::MeleeAttack()
{
	if(IsAttacking)
		return false;
	
	IsAttacking = true;
	GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage);

	return true;
}

void AZombieCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == AttackMontage)
	{
		IsAttacking = false;
	}
}

void AZombieCharacter::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!IsAttacking)
		return;

	if(TEXT("Attack") != GetMesh()->GetAnimInstance()->Montage_GetCurrentSection())
		return;
	
	AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(OtherActor);
	if(Character == nullptr)
		return;

	FVector HitDirection = Character->GetActorLocation() - GetActorLocation();
	HitDirection.Normalize();
	
	const FHitResult HitResult;
	
	UGameplayStatics::ApplyPointDamage(Character, AttackDamage, HitDirection, HitResult, GetInstigatorController(), this, nullptr);
}