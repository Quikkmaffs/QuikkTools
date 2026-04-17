using UnrealBuildTool;

public class QuikkToolsEditor : ModuleRules
{
    public QuikkToolsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "QuikkToolsRuntime"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Json",
                "LevelEditor",
                "Projects",
                "Slate",
                "SlateCore",
                "ToolMenus"
            }
        );
    }
}
