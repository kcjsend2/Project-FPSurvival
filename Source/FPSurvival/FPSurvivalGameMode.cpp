// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSurvivalGameMode.h"
#include "FPSurvivalCharacter.h"
#include "ZombieSpawner.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

AFPSurvivalGameMode::AFPSurvivalGameMode() : Super()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void AFPSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	ZombieSpawner = GetWorld()->SpawnActor<AZombieSpawner>(ZombieSpawnerClass, FVector().ZeroVector, FRotator().ZeroRotator);

	WaveReadyRemainTime = FTimespan::FromSeconds(WaveReadyDefaultTime);
	WaveProgressRemainTime = FTimespan::FromSeconds(WaveProgressDefaultTime);
}

void AFPSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 모든 액터들의 BeginPlay가 끝나면 호출함
	GetWorld()->OnWorldBeginPlay.AddUFunction(this, TEXT("InitCharacter"));
	WaveState = EWaveState::Ready;
}

void AFPSurvivalGameMode::InitCharacter()
{
	// 모든 플레이어에 웨이브 시작 정보 입력
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(It->Get()->GetCharacter());
		Character->SetMaxWaveInfo(MaxWave);
		Character->SetWaveReadyRemainTime(WaveReadyRemainTime);
		Character->OnWaveReady(CurrentWave);
	}
}

void AFPSurvivalGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(WaveState == EWaveState::Ready)
	{
		WaveReadyRemainTime -= FTimespan::FromSeconds(DeltaSeconds);
		for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(It->Get()->GetCharacter());
			Character->SetWaveReadyRemainTime(WaveReadyRemainTime);
		}

		if(WaveReadyRemainTime <= FTimespan::FromSeconds(0))
		{
			WaveReadyRemainTime = FTimespan::FromSeconds(0);
			WaveStart();
		}
	}
	if(WaveState == EWaveState::Progress)
	{
		WaveProgressRemainTime -= FTimespan::FromSeconds(DeltaSeconds);
		for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(It->Get()->GetCharacter());
			Character->SetWaveProgressRemainTime(WaveProgressRemainTime);
			Character->SetZombieCounter(ZombieSpawner->GetZombieCounter());
		}

		if(WaveProgressRemainTime <= FTimespan::FromSeconds(0))
		{
			WaveProgressRemainTime = FTimespan::FromSeconds(0);
			WaveEnd();
		}
	}
}

void AFPSurvivalGameMode::WaveStart()
{
	// 잘못된 실행임
	if(CurrentWave > MaxWave)
	{
		WaveState = EWaveState::Pause;
		return;
	}
	
	WaveState = EWaveState::Progress;
	WaveProgressRemainTime = FTimespan::FromSeconds(WaveProgressDefaultTime);
	
	// 1번 웨이브부터 시작함
	ZombieSpawner->SpawnZombieAtSpawnPoint(EnemyCountForWave[CurrentWave - 1], ZombieSpawnInterval);
	
	// 모든 플레이어에 웨이브 시작 정보 입력
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(It->Get()->GetCharacter());
		Character->OnWaveStart();
	}
}

void AFPSurvivalGameMode::WaveEnd()
{
	WaveState = EWaveState::Ready;
	WaveReadyRemainTime = FTimespan::FromSeconds(WaveReadyDefaultTime);
	
	CurrentWave++;

	// 모든 웨이브를 클리어
	if(CurrentWave > MaxWave)
	{
		// YOU WIN 표시, 최종 점수 표시, 타이틀로 돌아가기 버튼 표시
	}
	else
	{
		// 모든 플레이어에 웨이브 종료 정보 입력
		for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AFPSurvivalCharacter* Character = Cast<AFPSurvivalCharacter>(It->Get()->GetCharacter());
			Character->OnWaveReady(CurrentWave);
		}
	}
}
