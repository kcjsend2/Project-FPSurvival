// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "DeadMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UDeadMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	UButton* RestartButton;
	
	UPROPERTY(BlueprintReadWrite)
	UButton* TitleButton;
};
