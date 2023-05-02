#pragma once

#include "CoreMinimal.h"
#include "FPSurvivalCharacter.h"
#include <type_traits>

// T는 반드시 enum이어야 함
template<typename T>
class TStateMachine
{
	static_assert(std::is_enum_v<T>, "T must be enum type.");
	
public:
	TStateMachine() = delete;
	explicit TStateMachine(T InitialState);

	// 매 프레임 TransitionFunc를 검사하여 CurrentState가 StateFrom일 때,
	// TransitionFunc의 반환값이 True이면 StateTo로 CurrentState를 변경
	// 하나의 StateFrom에서 다수의 StateTo에 대한 동일한, 혹은 포함 관계에 있는 TransitionFunc를 넣지 말 것
	void AddTransitionFunc(std::function<void(AFPSurvivalCharacter&, bool)> TransitionFunc, T StateFrom, T StateTo);

	// CurrentState가 State로 바뀌었을 때 수행할 InitFunc을 추가하는 함수
	void AddInitFunc(std::function<void(AFPSurvivalCharacter&)> InitFunc, T State);

	// CurrentState가 State에서 다른 값으로 변경될 때 수행할 EndFunc을 추가하는 함수
	void AddEndFunc(std::function<void(AFPSurvivalCharacter&)> EndFunc, T State);

	// CurrentState에서 StateTo에 대한 검사
	bool CheckStateTransition(T StateTo);

	T GetCurrentState() const { return CurrentState; }
	T GetPrevState() const { return PrevState; }
	
private:
	T CurrentState;
	T PrevState;
	TMap<TPair<T, T>, std::function<bool(AFPSurvivalCharacter&)>> TransitionFuncTable;
	TMap<T, std::function<void(AFPSurvivalCharacter&)>> StateInitFuncTable;
	TMap<T, std::function<void(AFPSurvivalCharacter&)>> StateEndFuncTable;
};
