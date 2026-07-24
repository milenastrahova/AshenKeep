// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AshenKeep : ModuleRules
{
	public AshenKeep(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AshenKeep",
			"AshenKeep/Variant_Platforming",
			"AshenKeep/Variant_Platforming/Animation",
			"AshenKeep/Variant_Combat",
			"AshenKeep/Variant_Combat/AI",
			"AshenKeep/Variant_Combat/Animation",
			"AshenKeep/Variant_Combat/Gameplay",
			"AshenKeep/Variant_Combat/Interfaces",
			"AshenKeep/Variant_Combat/UI",
			"AshenKeep/Variant_SideScrolling",
			"AshenKeep/Variant_SideScrolling/AI",
			"AshenKeep/Variant_SideScrolling/Gameplay",
			"AshenKeep/Variant_SideScrolling/Interfaces",
			"AshenKeep/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
