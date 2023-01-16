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
}

template <typename StateEnum>
void TStateMachine<StateEnum>::AddOutFunc(FStateOut OutFunc, StateEnum State)
{
}

template <typename StateEnum>
void TStateMachine<StateEnum>::CheckStateTransition()
{
	for(int i = 0; i < TransitionFuncTable[CurrentState].Num(); ++i)
	{
		if(TransitionFuncTable[CurrentState][i].Execute())
		{
			
		}
	}
}

