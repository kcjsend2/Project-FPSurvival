// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSurvivalPlayerState.h"

void AFPSurvivalPlayerState::AddDamageDealt(float Damage)
{
	DamageDealt += Damage;
}

void AFPSurvivalPlayerState::AddDamageTaken(float Damage)
{
	DamageTaken += Damage;
}

void AFPSurvivalPlayerState::AddKillScore()
{
	KillScore++;
}
