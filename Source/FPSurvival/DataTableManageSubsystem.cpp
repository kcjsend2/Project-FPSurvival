// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTableManageSubsystem.h"
#include "UObject/ConstructorHelpers.h"

UDataTableManageSubsystem::UDataTableManageSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTable(TEXT("/Game/FirstPerson/DataTable/WeaponInitTable"));
	if (DataTable.Succeeded())
	{
		WeaponInitDataTable = DataTable.Object;
	}
}

FWeaponData* UDataTableManageSubsystem::GetWeaponInitData(int WeaponID) const
{
	FWeaponData* WeaponData = WeaponInitDataTable->FindRow<FWeaponData>(FName(*FString::FromInt(WeaponID)), FString(""));
	return WeaponData;
}
