// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void USoundManager::AddSound(FName Key, USoundCue* Sound)
{
	SoundMap[Key] = Sound;
}

void USoundManager::RemoveSound(FName Key)
{
	SoundMap.Remove(Key);
}

void USoundManager::PlaySound(FName Key, FVector Location, float Volume, float Pitch)
{
	UGameplayStatics::PlaySoundAtLocation(this, SoundMap[Key], Location, Volume, Pitch);
}

void USoundManager::PlaySoundByAudioComponent(FName Key)
{
	if(!SoundMap.Contains(Key))
		return;
		
	if(OwnerAudioComponent->IsPlaying())
		OwnerAudioComponent->Stop();
	
	OwnerAudioComponent->SetSound(SoundMap[Key]);
	OwnerAudioComponent->Play();
}
