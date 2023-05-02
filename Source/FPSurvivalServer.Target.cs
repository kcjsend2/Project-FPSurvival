// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class FPSurvivalServerTarget : TargetRules
{
	public FPSurvivalServerTarget(TargetInfo Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("FPSurvival");
	}
}
