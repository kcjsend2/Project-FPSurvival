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

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
public:
	UPROPERTY(BlueprintReadWrite)
	double Spread;
	
	UPROPERTY(BlueprintReadWrite)
	double Thickness = 5;
	
	UPROPERTY(BlueprintReadWrite)
	double Length = 25;

	float FireSpreadValue = 0;
	
	UPROPERTY(EditAnywhere)
	float SpreadDecrease;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HitIndicatorColor = {1, 1, 1, 0};

	// 초당 히트 인디케이터의 알파가 줄어드는 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitIndicatorAlphaDecrease;

};
