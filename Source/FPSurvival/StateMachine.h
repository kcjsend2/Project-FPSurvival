// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FStateTransition);
DECLARE_DELEGATE(FStateInit);
DECLARE_DELEGATE(FStateOut)

template <typename StateEnum>
class FPSURVIVAL_API TStateMachine
{
public:
	explicit TStateMachine(StateEnum InitialState);
	~TStateMachine();

	// 매 프레임 TransitionFunc를 검사하여 CurrentState가 StateFrom일 때,
	// TransitionFunc의 반환값이 True이면 StateTo로 CurrentState를 변경
	// 하나의 StateFrom에서 다수의 StateTo에 대한 동일한, 혹은 포함 관계에 있는 TransitionFunc를 넣지 말 것
	void AddTransitionFunc(FStateTransition TransitionFunc, StateEnum StateFrom, StateEnum StateTo);

	// CurrentState가 State로 바뀌었을 때 수행할 InitFunc을 추가하는 함수
	void AddInitFunc(FStateInit InitFunc, StateEnum State);

	// CurrentState가 State에서 다른 값으로 변경될 때 수행할 OutFunc을 추가하는 함수
	void AddOutFunc(FStateOut OutFunc, StateEnum State);

	// 액터의 Tick에서 반드시 호출해야함
	// CurrentState에서 다른 State로 변경될 수 있는지 검사하는 함수
	void CheckStateTransition();

	
	
private:
	StateEnum CurrentState;
	TMap<StateEnum, TArray<StateEnum>> StateConnected;
	TMap<StateEnum, TArray<FStateTransition>> TransitionFuncTable;
	TMap<StateEnum, FStateInit> StateInitFuncTable;
	TMap<StateEnum, FStateOut> StateOutFuncTable;
};
