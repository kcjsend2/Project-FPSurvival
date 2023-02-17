// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_Detect.h"

#include "AIController.h"
#include "EnemyCharacter.h"
#include "FPSurvivalCharacter.h"
#include "ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_Detect::UBTService_Detect()
{
	NodeName = TEXT("Detect");
	Interval = 1.0f;
}

void UBTService_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if(ControllingPawn == nullptr)
		return;

	FVector Center = ControllingPawn->GetActorLocation();

	if(GetWorld() == nullptr)
		return;
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParams(NAME_None, false, ControllingPawn);
	
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
	
	AEnemyCharacter* ControllingEnemy = Cast<AEnemyCharacter>(ControllingPawn);

	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AZombieAIController::IsAttackingKey, ControllingEnemy->IsAttacking);
	
	// 공격 가능한 사거리 안에 있는가?
	if(Character == nullptr)
		return;

	if(ControllingEnemy == nullptr)
		return;

	float Distance = Character->GetDistanceTo(ControllingEnemy);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AZombieAIController::CanAttackKey,
		Distance <= ControllingEnemy->AttackRange && !ControllingEnemy->IsAttacking);

	
}
