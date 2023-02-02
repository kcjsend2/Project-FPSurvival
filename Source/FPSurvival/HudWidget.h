// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HudWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UHudWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CurrentWeaponImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HPPercentage = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaPercentage = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TotalAmmo;
	
};
