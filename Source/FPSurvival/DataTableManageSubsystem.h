// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomDataTables.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataTableManageSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FPSURVIVAL_API UDataTableManageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UDataTableManageSubsystem();

public:
	FWeaponData* GetWeaponInitData(int WeaponID);
	
protected:
	UPROPERTY()
	UDataTable* WeaponInitDataTable;
};
