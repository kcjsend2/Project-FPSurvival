// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "WinMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UWinMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnTitleButtonClicked();
	
	UPROPERTY(BlueprintReadWrite)
	int KillCount;
	
	UPROPERTY(BlueprintReadWrite)
	int DamageDealt;
	
	UPROPERTY(BlueprintReadWrite)
	int DamageTaken;
	
public:
	UFUNCTION()
	void SetKillCount(int KillCountParam);
	
	UFUNCTION()
	void SetDamageDealt(int DamageDealtParam);
	
	UFUNCTION()
	void SetDamageTaken(int DamageTakenParam);

	UFUNCTION()
	void SetHidden();
	
	UFUNCTION()
	void SetVisible();
	
	UPROPERTY(BlueprintReadWrite)
	UButton* TitleButton;
};
