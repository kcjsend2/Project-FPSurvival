// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MeleeAttack.h"
#include "AIController.h"
#include "ZombieCharacter.h"

UBTTask_MeleeAttack::UBTTask_MeleeAttack()
{
	NodeName = TEXT("MeleeAttack");
}

EBTNodeResult::Type UBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	AZombieCharacter* ControllingPawn = Cast<AZombieCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if(ControllingPawn == nullptr)
		return EBTNodeResult::Failed;

	return ControllingPawn->MeleeAttack() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
