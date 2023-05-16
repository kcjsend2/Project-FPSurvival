// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponBase.h"
#include "CustomDataTables.generated.h"


USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FWeaponData(): MagazineLimit(0), BulletDamage(0), RecoilYaw(0), RecoilPitch(0), FireMode(), ReloadType()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int MagazineLimit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int BulletDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float RecoilYaw;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float RecoilPitch;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FName FPSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FName TPSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	EFireMode FireMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	EReloadType ReloadType;
};