// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "DeadMenuWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnGameRestart);

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UDeadMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	// UFUNCTION()
	// void OnRestartButtonClicked();

	UFUNCTION()
	void OnTitleButtonClicked();

	// 보류
	// UPROPERTY(BlueprintReadWrite)
	// UButton* RestartButton;
	
	UPROPERTY(BlueprintReadWrite)
	UButton* TitleButton;

public:
	// SetVisibility를 델리게이트로 이용하기 위해 정의... 별로 이렇게 하고 싶지는 않았다.
	UFUNCTION()
	void SetHidden();
	
	UFUNCTION()
	void SetVisible();

	FOnGameRestart OnGameRestart;
};
