// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "TitleScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UTitleScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnStartButtonClicked();

public:
	UPROPERTY(BlueprintReadWrite)
	UButton* StartButton;
	
	UPROPERTY(BlueprintReadWrite)
	UButton* ExitButton;
};
