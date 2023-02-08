// Fill out your copyright notice in the Description page of Project Settings.


#include "CrossHairWidget.h"

void UCrossHairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if(FireSpreadValue > 0)
	{
		FireSpreadValue -= SpreadDecrease * InDeltaTime;
		Spread -= SpreadDecrease * InDeltaTime;

		if(FireSpreadValue < 0)
		{
			Spread -= FireSpreadValue;
			FireSpreadValue = 0;
		}
	}
}