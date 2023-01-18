// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementStateMachine.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FStateTransition);
DECLARE_DYNAMIC_DELEGATE(FStateInit);
DECLARE_DYNAMIC_DELEGATE(FStateEnd);

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Walking,
	Sprinting,
	Crouching,
	Sliding
};

UCLASS()
class FPSURVIVAL_API UMovementStateMachine : public UObject
{
	GENERATED_BODY()
	
public:
	UMovementStateMachine();

	// 매 프레임 TransitionFunc를 검사하여 CurrentState가 StateFrom일 때,
	// TransitionFunc의 반환값이 True이면 StateTo로 CurrentState를 변경
	// 하나의 StateFrom에서 다수의 StateTo에 대한 동일한, 혹은 포함 관계에 있는 TransitionFunc를 넣지 말 것
	void AddTransitionFunc(FStateTransition TransitionFunc, EMovementState StateFrom, EMovementState StateTo);

	// CurrentState가 State로 바뀌었을 때 수행할 InitFunc을 추가하는 함수
	void AddInitFunc(FStateInit InitFunc, EMovementState State);

	// CurrentState가 State에서 다른 값으로 변경될 때 수행할 EndFunc을 추가하는 함수
	void AddEndFunc(FStateEnd EndFunc, EMovementState State);

	// CurrentState에서 StateTo에 대한 검사
	bool CheckStateTransition(EMovementState StateTo);

	EMovementState GetCurrentState() const { return CurrentState; }
	EMovementState GetPrevState() const { return PrevState; }
	
private:
	EMovementState CurrentState;
	EMovementState PrevState;
	TMap<TPair<EMovementState, EMovementState>, FStateTransition> TransitionFuncTable;
	TMap<EMovementState, FStateInit> StateInitFuncTable;
	TMap<EMovementState, FStateEnd> StateEndFuncTable;
};
