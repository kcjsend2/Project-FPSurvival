// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementStateMachine.h"

UMovementStateMachine::UMovementStateMachine()
{
	CurrentState = EMovementState::Walking;
	PrevState = EMovementState::Walking;
}

void UMovementStateMachine::AddTransitionFunc(FStateTransition TransitionFunc, EMovementState StateFrom,
	EMovementState StateTo)
{
	TransitionFuncTable.Add(TPair<EMovementState, EMovementState>(StateFrom, StateTo), TransitionFunc);
}

void UMovementStateMachine::AddInitFunc(FStateInit InitFunc, EMovementState State)
{
	StateInitFuncTable.Add(State, InitFunc);
}

void UMovementStateMachine::AddEndFunc(FStateEnd EndFunc, EMovementState State)
{
	StateEndFuncTable.Add(State, EndFunc);
}

bool UMovementStateMachine::CheckStateTransition(EMovementState StateTo)
{
	if(CurrentState == StateTo)
		return false;

	if(!TransitionFuncTable.Contains(TPair<EMovementState, EMovementState>(CurrentState, StateTo)))
		return false;
	
	if(!TransitionFuncTable[TPair<EMovementState, EMovementState>(CurrentState, StateTo)].IsBound())
		return false;
	
	if(TransitionFuncTable[TPair<EMovementState, EMovementState>(CurrentState, StateTo)].Execute())
	{
		PrevState = CurrentState;
		CurrentState = StateTo;
		
		StateEndFuncTable[PrevState].ExecuteIfBound();
		StateInitFuncTable[CurrentState].ExecuteIfBound();
		return true;
	}
	return false;
}

