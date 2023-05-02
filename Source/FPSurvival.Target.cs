// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class FPSurvivalTarget : TargetRules
{
	public FPSurvivalTarget(TargetInfo Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("FPSurvival");
	}
}
