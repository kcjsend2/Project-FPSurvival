// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FPSurvival : ModuleRules
{
	public FPSurvival(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG", "AnimGraphRuntime" });
	}
}
