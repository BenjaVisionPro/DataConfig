using UnrealBuildTool;

public class DataConfigEngineExtra5 : ModuleRules
{
	public DataConfigEngineExtra5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"DataConfigCore",
			"DataConfigExtra",
			"StructUtils",
			});
	}
}
