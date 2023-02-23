// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_ZombieDetectPlayer.h"

#include "AIController.h"
#include "EnemyCharacter.h"
#include "FPSurvivalCharacter.h"
#include "ZombieAIController.h"
#include "ZombieCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

class AZombieCharacter;

UBTService_ZombieDetectPlayer::UBTService_ZombieDetectPlayer()
{
	NodeName = TEXT("Zombie Detect Player");
	Interval = 1.0f;
}

void UBTService_ZombieDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if(ControllingPawn == nullptr)
		return;

	FVector Center = ControllingPawn->GetActorLocation();
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(AZombieAIController::ForwardKey,
		Center + ControllingPawn->GetActorForwardVector() * 100);
	
	if(GetWorld() == nullptr)
		return;
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParams(NAME_None, false, ControllingPawn);
	
	AFPSurvivalCharacter* Character = nullptr;
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
					if(Character->GetDistanceTo(ControllingPawn) > NewCharacter->GetDistanceTo(ControllingPawn))
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
	
	OwnerComp.GetBlackboardComponent()->SetValueAsObject(AZombieAIController::PlayerCharacterKey, Character);
	
	AZombieCharacter* ControllingZombie = Cast<AZombieCharacter>(ControllingPawn);

	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AZombieAIController::IsAttackingKey, ControllingZombie->IsAttacking);
	
	// 공격 가능한 사거리 안에 있는가?
	if(Character == nullptr)
		return;

	if(ControllingZombie == nullptr)
		return;

	ControllingZombie->BeginSprint();
	
	float Distance = Character->GetDistanceTo(ControllingZombie);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AZombieAIController::CanAttackKey,
		Distance <= ControllingZombie->AttackRange && !ControllingZombie->IsAttacking);

	
}
