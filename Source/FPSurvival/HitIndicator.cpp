// Fill out your copyright notice in the Description page of Project Settings.


#include "HitIndicator.h"

#include "Kismet/KismetMathLibrary.h"

void UHitIndicator::NativeConstruct()
{
	Super::NativeConstruct();
	IndicatorMaterial = Border->GetDynamicMaterial();
	CurrentColor = FLinearColor(1, 0, 0, 1);
}

void UHitIndicator::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	CurrentColor.A -= InDeltaTime * AlphaDecrease;
	if(CurrentColor.A <= 0)
	{
		RemoveFromParent();
		return;
	}
	
	IndicatorMaterial->SetVectorParameterValue("Color", CurrentColor);

	const FVector ForwardVector = Character->GetActorForwardVector();

	//Degree로 변환
	float Angle = 180.0 / UE_DOUBLE_PI * FMath::Atan2(ForwardVector.X * HitDirection.Y - ForwardVector.Y * HitDirection.X, ForwardVector.Dot(HitDirection));
	Angle = FMath::GetMappedRangeValueClamped(FVector2d(180, -180), FVector2d(0, 1), Angle);
	
	IndicatorMaterial->SetScalarParameterValue("Angle", Angle);
}
