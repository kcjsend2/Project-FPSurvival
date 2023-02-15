// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAIController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

const FName AZombieAIController::PlayerCharacterKey(TEXT("PlayerCharacter"));

AZombieAIController::AZombieAIController()
{
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UBlackboardComponent* BlackBoardLocal = GetBlackboardComponent();
	if(UseBlackboard(BBAsset,BlackBoardLocal))
	{
		if(!RunBehaviorTree(BTAsset))
			UE_LOG(LogTemp, Log, TEXT("Zombie AI Error!"));
	}
}