// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrossHairWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UCrossHairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	double Spread;
	
	UPROPERTY(BlueprintReadWrite)
	double Thickness = 5;
	
	UPROPERTY(BlueprintReadWrite)
	double Length = 25;
};
