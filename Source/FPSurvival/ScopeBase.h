// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScopeBase.generated.h"

UCLASS()
class FPSURVIVAL_API AScopeBase : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleDefaultsOnly, Category="Scope", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Scope", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ScopeMesh;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Scope", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* CaptureComponent;
	
public:
	// Sets default values for this actor's properties
	AScopeBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
