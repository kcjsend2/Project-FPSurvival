// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPlayGameMode.h"
#include "CustomDataTables.h"
#include "UObject/ConstructorHelpers.h"

AMultiPlayGameMode::AMultiPlayGameMode()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTable(TEXT("/Game/FirstPerson/DataTable/WeaponInitTable"));
	if (DataTable.Succeeded())
	{
		WeaponInitDataTable = DataTable.Object;
	}
}

void AMultiPlayGameMode::BeginPlay()
{
	Super::BeginPlay();
}
