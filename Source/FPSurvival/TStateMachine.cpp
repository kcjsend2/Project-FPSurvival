// Fill out your copyright notice in the Description page of Project Settings.


#include "TStateMachine.h"

template<typename T, typename O>
TStateMachine<T, O>::TStateMachine(T InitialState, O* InitialOwner)
{
	CurrentState = InitialState;
	PrevState = InitialState;
	Owner = InitialOwner;
}

template<typename T, typename O>
void TStateMachine<T, O>::AddTransitionFunc(std::function<bool(O&)> TransitionFunc, T StateFrom, T StateTo)
{
	TransitionFuncTable.Add(TPair<T, T>(StateFrom, StateTo), TransitionFunc);
}

template<typename T, typename O>
void TStateMachine<T, O>::AddInitFunc(std::function<void(O&)> InitFunc, T State)
{
	StateInitFuncTable.Add(State, InitFunc);
}

template<typename T, typename O>
void TStateMachine<T, O>::AddEndFunc(std::function<void(O&)> EndFunc, T State)
{
	StateEndFuncTable.Add(State, EndFunc);
}

template<typename T, typename O>
bool TStateMachine<T, O>::CheckStateTransition(T StateTo)
{
	if(CurrentState == StateTo)
		return false;

	if(!TransitionFuncTable.Contains(TPair<T, T>(CurrentState, StateTo)))
		return false;
	
	if(!TransitionFuncTable[TPair<T, T>(CurrentState, StateTo)])
		return false;
	
	if(TransitionFuncTable[TPair<T, T>(CurrentState, StateTo)]())
	{
		PrevState = CurrentState;
		CurrentState = StateTo;

		if(StateEndFuncTable[PrevState])
			StateEndFuncTable[PrevState](Owner);
		
		if(StateInitFuncTable[CurrentState])
			StateInitFuncTable[CurrentState](Owner);
		return true;
	}
	return false;
}
