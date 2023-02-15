// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_Detect.h"

#include "AIController.h"
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
	float DetectRadius = 600.0f;

	if(GetWorld() == nullptr)
		return;
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParams(NAME_None, false, ControllingPawn);
	bool Hit = GetWorld()->OverlapMultiByChannel(OverlapResults, Center, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(DetectRadius), CollisionQueryParams);

	if(Hit)
	{
		for(auto const& OverlapResult : OverlapResults)
		{
			AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(OverlapResult.GetActor());
			if(Character && Character->GetController()->IsPlayerController())
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsObject(AZombieAIController::PlayerCharacterKey, Character);
				return;
			}
		}
	}
}
