// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_ZombieDetectPlayer.h"

#include "AIController.h"
#include "EnemyCharacter.h"
#include "FPSurvivalCharacter.h"
#include "ZombieAIController.h"
#include "ZombieCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

class AZombieCharacter;

UBTService_ZombieDetectPlayer::UBTService_ZombieDetectPlayer()
{
	NodeName = TEXT("Zombie Detect Player");
	Interval = 1.0f;
}

void UBTService_ZombieDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AZombieCharacter* ControllingCharacter = Cast<AZombieCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if(ControllingCharacter == nullptr)
		return;
	
	if(GetWorld() == nullptr)
		return;
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParams(NAME_None, false, ControllingCharacter);
	
	AFPSurvivalCharacter* Character = ControllingCharacter->GetTargetCharacter();
	FVector Center = ControllingCharacter->GetActorLocation();
	
	if(Character == nullptr)
	{
		// 코옵 플레이 감안
		if(GetWorld()->OverlapMultiByChannel(OverlapResults, Center, FQuat::Identity,
			ECC_GameTraceChannel1, FCollisionShape::MakeSphere(DetectRadius), CollisionQueryParams))
		{
			for(auto const& OverlapResult : OverlapResults)
			{
				AFPSurvivalCharacter* NewCharacter = Cast<AFPSurvivalCharacter>(OverlapResult.GetActor());
				if(NewCharacter && NewCharacter->GetController()->IsPlayerController())
				{
					if(Character != nullptr)
					{
						if(Character->GetDistanceTo(ControllingCharacter) > NewCharacter->GetDistanceTo(ControllingCharacter))
						{
							Character = NewCharacter;
						}
					}
					else
					{
						Character = NewCharacter;
					}
				}
			}
		}
		ControllingCharacter->SetTargetCharacter(Character);
	}
	OwnerComp.GetBlackboardComponent()->SetValueAsObject(AZombieAIController::PlayerCharacterKey, Character);
	
	AZombieCharacter* ControllingZombie = Cast<AZombieCharacter>(ControllingCharacter);

	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AZombieAIController::IsAttackingKey, ControllingZombie->IsAttacking);

	// 벽에 부딪히면 반사각의 방향으로 전진
	if(Character == nullptr)
	{
		FVector ForwardTarget = Center + ControllingCharacter->GetActorForwardVector() * 100;
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, Center, ForwardTarget, ECC_Visibility, CollisionQueryParams);

		FVector ReflectionVector = FMath::GetReflectionVector(ControllingCharacter->GetActorForwardVector(), HitResult.ImpactNormal);
		ForwardTarget = Center + ReflectionVector * 100;
		
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(AZombieAIController::ForwardKey, ForwardTarget);
		return;
	}

	if(ControllingZombie == nullptr)
		return;

	ControllingZombie->BeginSprint();
	
	// 공격 가능한 사거리 안에 있는가?
	float Distance = Character->GetDistanceTo(ControllingZombie);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AZombieAIController::CanAttackKey,
		Distance <= ControllingZombie->AttackRange && !ControllingZombie->IsAttacking);

	
}
