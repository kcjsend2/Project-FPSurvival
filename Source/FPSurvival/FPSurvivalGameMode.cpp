// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSurvivalGameMode.h"
#include "FPSurvivalCharacter.h"
#include "ZombieSpawner.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

AFPSurvivalGameMode::AFPSurvivalGameMode() : Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void AFPSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	ZombieSpawner = GetWorld()->SpawnActor<AZombieSpawner>(ZombieSpawnerClass, FVector().ZeroVector, FRotator().ZeroRotator);
}
