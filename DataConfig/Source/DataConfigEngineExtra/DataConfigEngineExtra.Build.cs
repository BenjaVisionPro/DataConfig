using UnrealBuildTool;

public class DataConfigEngineExtra : ModuleRules
{
	public DataConfigEngineExtra(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"MessageLog",
			"GameplayTags",
			"GameplayAbilities",
			"DataConfigCore",
			"DataConfigExtra",
#if UE_5_0_OR_LATER
			"StructUtils",
			"DataConfigEngineExtra5",
#endif
			});

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
				}
			);
		}
	}
}
