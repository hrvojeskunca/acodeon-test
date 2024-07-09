// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MidProgrammerTest : ModuleRules
{
	public MidProgrammerTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });
	}
}
