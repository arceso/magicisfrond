// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Moon_Abyss : ModuleRules
{
	public Moon_Abyss(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });
	}
}
