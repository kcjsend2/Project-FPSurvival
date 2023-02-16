// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAnimInstance.h"

void UZombieAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<AZombieCharacter>(TryGetPawnOwner());
}

void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(OwningCharacter != nullptr)
	{
		IsMoving = OwningCharacter->GetVelocity().Size() > 0;
		IsSprinting = OwningCharacter->GetVelocity().Size() > 10;
	}
}
