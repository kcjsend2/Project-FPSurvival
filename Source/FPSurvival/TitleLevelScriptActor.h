// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LoadingScreenWidget.h"
#include "TitleScreenWidget.h"
#include "Engine/LevelScriptActor.h"
#include "TitleLevelScriptActor.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API ATitleLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

public:
	ATitleLevelScriptActor();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	TSubclassOf<UTitleScreenWidget> TitleScreenClass;
	TSubclassOf<ULoadingScreenWidget> LoadingScreenClass;

	UPROPERTY()
	UTitleScreenWidget* TitleScreenWidget;
	
	UPROPERTY()
	ULoadingScreenWidget* LoadingScreenWidget;

	UFUNCTION()
	void OnStartButtonClicked();
};
