// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PickUpWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UPickUpWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PickUpWeaponImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PickUpWeaponName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PickUpGauge = 0;
};
