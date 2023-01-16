// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachine.h"

template <typename StateEnum>
TStateMachine<StateEnum>::TStateMachine(StateEnum InitialState)
{
	CurrentState = InitialState;
}

template <typename StateEnum>
TStateMachine<StateEnum>::~TStateMachine()
{
}

template <typename StateEnum>
void TStateMachine<StateEnum>::AddTransitionFunc(FStateTransition TransitionFunc, StateEnum StateFrom,
	StateEnum StateTo)
{
	StateConnected[StateFrom].Add(StateTo);
	TransitionFuncTable[StateFrom] = TransitionFunc;
}

template <typename StateEnum>
void TStateMachine<StateEnum>::AddInitFunc(FStateInit InitFunc, StateEnum State)
{
	StateInitFuncTable[State] = InitFunc;
}

template <typename StateEnum>
void TStateMachine<StateEnum>::AddOutFunc(FStateOut OutFunc, StateEnum State)
{
	StateOutFuncTable[State] = OutFunc;
}

template <typename StateEnum>
void TStateMachine<StateEnum>::CheckStateTransition()
{
	for(TPair<StateEnum, FStateTransition>& TransitionInfo : TransitionFuncTable[CurrentState])
	{
		if(TransitionInfo.Value.Execute())
		{
			CurrentState = TransitionInfo.Key;
			break;
		}
	}
}

template <typename StateEnum>
void TStateMachine<StateEnum>::CheckStateTransition(StateEnum StateTo)
{
	if(!TransitionFuncTable[CurrentState][StateTo].IsBound())
	{
		return;
	}
	
	if(TransitionFuncTable[CurrentState][StateTo].Execute())
	{
		CurrentState = StateTo;
	}
}

