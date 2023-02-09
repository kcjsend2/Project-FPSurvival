// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSurvivalCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "HitIndicator.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UHitIndicator : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UMaterialInstanceDynamic* IndicatorMaterial;
	
	UPROPERTY(BlueprintReadWrite)
	UBorder* Border;

	UPROPERTY()
	AFPSurvivalCharacter* Character;
	FVector HitDirection;
	FLinearColor CurrentColor;

	UPROPERTY(EditAnywhere)
	float AlphaDecrease;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
