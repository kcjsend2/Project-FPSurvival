// Fill out your copyright notice in the Description page of Project Settings.


#include "TStateMachine.h"

template <typename T>
TStateMachine<T>::TStateMachine()
{
	CurrentState = T();
	PrevState = T();
}

template <typename T>
void TStateMachine<T>::AddTransitionFunc(std::function<void(AFPSurvivalCharacter&, bool)> TransitionFunc, T StateFrom, T StateTo)
{
	TransitionFuncTable.Add(TPair<T, T>(StateFrom, StateTo), TransitionFunc);
}

template <typename T>
void TStateMachine<T>::AddInitFunc(std::function<void(AFPSurvivalCharacter&)> InitFunc, T State)
{
	StateInitFuncTable.Add(State, InitFunc);
}

template <typename T>
void TStateMachine<T>::AddEndFunc(std::function<void(AFPSurvivalCharacter&)> EndFunc, T State)
{
	StateEndFuncTable.Add(State, EndFunc);
}

template <typename T>
bool TStateMachine<T>::CheckStateTransition(T StateTo)
{
	if(CurrentState == StateTo)
		return false;

	if(!TransitionFuncTable.Contains(TPair<T, T>(CurrentState, StateTo)))
		return false;
	
	if(!TransitionFuncTable[TPair<T, T>(CurrentState, StateTo)])
		return false;
	
	if(TransitionFuncTable[TPair<T, T>(CurrentState, StateTo)].Execute())
	{
		PrevState = CurrentState;
		CurrentState = StateTo;
		
		StateEndFuncTable[PrevState].ExecuteIfBound();
		StateInitFuncTable[CurrentState].ExecuteIfBound();
		return true;
	}
	return false;
}
