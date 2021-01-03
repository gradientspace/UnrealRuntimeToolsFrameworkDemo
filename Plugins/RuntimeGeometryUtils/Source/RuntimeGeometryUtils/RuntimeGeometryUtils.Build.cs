// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RuntimeGeometryUtils : ModuleRules
{
	public RuntimeGeometryUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GeometricObjects",
				"DynamicMesh",
				"ProceduralMeshComponent"
			}
			);

        if ((Target.Platform == UnrealTargetPlatform.Win64)) {
			PublicDependencyModuleNames.Add("ModelingComponents");
        }


        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"MeshDescription",
				"StaticMeshDescription",
				"GeometryAlgorithms",
				"MeshConversion"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
