// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleLevelScriptActor.h"

#include "LevelLoadingGameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"

ATitleLevelScriptActor::ATitleLevelScriptActor()
{
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FClassFinder<UTitleScreenWidget> TitleScreenClassFinder(TEXT("/Game/FirstPerson/Widgets/WBTitleScreen.WBTitleScreen_C"));
	TitleScreenClass = TitleScreenClassFinder.Class;
	static ConstructorHelpers::FClassFinder<ULoadingScreenWidget> LoadingScreenClassFinder(TEXT("/Game/FirstPerson/Widgets/WBLoadingScreen.WBLoadingScreen_C"));
	LoadingScreenClass = LoadingScreenClassFinder.Class;
}

void ATitleLevelScriptActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(true);
	
	const UWorld* World = GetWorld();
	TitleScreenWidget = CreateWidget<UTitleScreenWidget>(GetWorld(), TitleScreenClass);
	if(TitleScreenWidget != nullptr)
	{
		TitleScreenWidget->AddToViewport();
		TitleScreenWidget->StartButton->OnClicked.AddDynamic(this, &ATitleLevelScriptActor::OnStartButtonClicked);
	}
	
	LoadingScreenWidget = CreateWidget<ULoadingScreenWidget>(GetWorld(), LoadingScreenClass);
	if(LoadingScreenWidget != nullptr)
	{
		LoadingScreenWidget->AddToViewport();
		LoadingScreenWidget->SetVisibility(ESlateVisibility::Hidden);
	}

}

void ATitleLevelScriptActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(TitleScreenWidget->bIsLoading)
	{
		LoadingScreenWidget->LoadingProgress =
		GetGameInstance()->GetSubsystem<ULevelLoadingGameInstanceSubsystem>()->GetLevelLoadProgress();
	}
}

void ATitleLevelScriptActor::OnStartButtonClicked()
{
	LoadingScreenWidget->SetVisibility(ESlateVisibility::Visible);
	TitleScreenWidget->SetVisibility(ESlateVisibility::Hidden);
}
