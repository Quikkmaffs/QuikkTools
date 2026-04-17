#include "QuikkProjectLogLibrary.h"

#include "Brushes/SlateImageBrush.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "LevelEditor.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenus.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Notifications/SNotificationList.h"

class FQuikkToolsEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		RegisterStyle();
		PluginCommands = MakeShared<FUICommandList>();

		ExportCommand = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("QuikkTools.ExportCurrentProjectLog"),
			TEXT("Copies Saved/Logs/<Project>.log into the project's Exports folder."),
			FConsoleCommandDelegate::CreateRaw(this, &FQuikkToolsEditorModule::RunExport),
			ECVF_Default
		);

		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FQuikkToolsEditorModule::RegisterMenus)
		);

		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		ToolbarExtender = MakeShared<FExtender>();
		ToolbarExtender->AddToolBarExtension(
			TEXT("Play"),
			EExtensionHook::After,
			PluginCommands,
			FToolBarExtensionDelegate::CreateRaw(this, &FQuikkToolsEditorModule::AddToolbarExtension)
		);
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);

		if (ExportCommand != nullptr)
		{
			IConsoleManager::Get().UnregisterConsoleObject(ExportCommand);
			ExportCommand = nullptr;
		}

		UnregisterStyle();
	}

private:
	static constexpr const TCHAR* ToolbarOptionsMenuName = TEXT("QuikkTools.ToolbarOptions");

	void RegisterStyle()
	{
		if (StyleSet.IsValid())
		{
			return;
		}

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("QuikkTools"));
		if (!Plugin.IsValid())
		{
			return;
		}

		const TSharedRef<FSlateStyleSet> NewStyleSet = MakeShared<FSlateStyleSet>(TEXT("QuikkToolsStyle"));
		NewStyleSet->SetContentRoot(Plugin->GetBaseDir());
		NewStyleSet->Set(
			TEXT("QuikkTools.ToolbarIcon"),
			new FSlateImageBrush(NewStyleSet->RootToContentDir(TEXT("quikkanchor-app-icon"), TEXT(".png")), FVector2D(20.0f, 20.0f))
		);
		NewStyleSet->Set(
			TEXT("QuikkTools.MenuIcon"),
			new FSlateImageBrush(NewStyleSet->RootToContentDir(TEXT("quikkanchor-app-icon"), TEXT(".png")), FVector2D(16.0f, 16.0f))
		);

		FSlateStyleRegistry::RegisterSlateStyle(*NewStyleSet);
		StyleSet = NewStyleSet;
	}

	void UnregisterStyle()
	{
		if (!StyleSet.IsValid())
		{
			return;
		}

		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		StyleSet.Reset();
	}

	FSlateIcon GetQuikkToolsMenuIcon() const
	{
		if (StyleSet.IsValid())
		{
			return FSlateIcon(StyleSet->GetStyleSetName(), TEXT("QuikkTools.MenuIcon"));
		}

		return FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Save"));
	}

	FSlateIcon GetQuikkToolsToolbarIcon() const
	{
		if (StyleSet.IsValid())
		{
			return FSlateIcon(StyleSet->GetStyleSetName(), TEXT("QuikkTools.ToolbarIcon"));
		}

		return FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Save"));
	}

	void AddExportLogMenuEntry(FToolMenuSection& Section, const FToolUIActionChoice& ExportAction) const
	{
		Section.AddMenuEntry(
			TEXT("QuikkTools.ExportCurrentProjectLog"),
			FText::FromString(TEXT("Export Current Project Log")),
			FText::FromString(TEXT("Copy Saved/Logs/<Project>.log into the project's Exports folder.")),
			GetQuikkToolsMenuIcon(),
			ExportAction
		);
	}

	void AddComingSoonMenuEntry(
		FToolMenuSection& Section,
		const FName EntryName,
		const TCHAR* Label,
		const TCHAR* ToolTip
	) const
	{
		const FUIAction DisabledAction(
			FExecuteAction(),
			FCanExecuteAction::CreateLambda([]()
			{
				return false;
			})
		);

		Section.AddMenuEntry(
			EntryName,
			FText::FromString(Label),
			FText::FromString(ToolTip),
			FSlateIcon(),
			DisabledAction
		);
	}

	void PopulateToolbarDropdownMenu(UToolMenu* InMenu)
	{
		const FToolUIActionChoice ExportAction(FExecuteAction::CreateRaw(this, &FQuikkToolsEditorModule::RunExport));

		FToolMenuSection& LogsSection = InMenu->FindOrAddSection(TEXT("QuikkToolsLogs"));
		LogsSection.Label = FText::FromString(TEXT("Logs"));
		AddExportLogMenuEntry(LogsSection, ExportAction);

		FToolMenuSection& DiagnosticsSection = InMenu->FindOrAddSection(TEXT("QuikkToolsDiagnostics"));
		DiagnosticsSection.Label = FText::FromString(TEXT("Diagnostics"));
		AddComingSoonMenuEntry(
			DiagnosticsSection,
			TEXT("QuikkTools.DiagnosticsComingSoon"),
			TEXT("More diagnostics tools coming soon"),
			TEXT("Future QuikkTools diagnostics features will appear in this section.")
		);

		FToolMenuSection& UtilitiesSection = InMenu->FindOrAddSection(TEXT("QuikkToolsUtilities"));
		UtilitiesSection.Label = FText::FromString(TEXT("Utilities"));
		AddComingSoonMenuEntry(
			UtilitiesSection,
			TEXT("QuikkTools.UtilitiesComingSoon"),
			TEXT("More utility tools coming soon"),
			TEXT("Future QuikkTools utility features will appear in this section.")
		);
	}

	TSharedRef<SWidget> CreateToolbarEntryMenu()
	{
		if (UToolMenus::TryGet() == nullptr)
		{
			return SNullWidget::NullWidget;
		}

		return UToolMenus::Get()->GenerateWidget(ToolbarOptionsMenuName, FToolMenuContext());
	}

	void AddToolbarExtension(FToolBarBuilder& Builder)
	{
		Builder.SetLabelVisibility(EVisibility::All);
		Builder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FQuikkToolsEditorModule::CreateToolbarEntryMenu),
			FText::FromString(TEXT("QuikkTools")),
			FText::FromString(TEXT("Open QuikkTools to access logs, diagnostics, and utilities.")),
			GetQuikkToolsToolbarIcon(),
			false
		);
	}

	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);
		const FToolUIActionChoice ExportAction(FExecuteAction::CreateRaw(this, &FQuikkToolsEditorModule::RunExport));

		UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
		FToolMenuSection& Section = ToolsMenu->FindOrAddSection(TEXT("QuikkTools"));
		AddExportLogMenuEntry(Section, ExportAction);

		UToolMenu* ToolbarOptionsMenu = nullptr;
		if (UToolMenus::Get()->IsMenuRegistered(ToolbarOptionsMenuName))
		{
			ToolbarOptionsMenu = UToolMenus::Get()->ExtendMenu(ToolbarOptionsMenuName);
		}
		else
		{
			ToolbarOptionsMenu = UToolMenus::Get()->RegisterMenu(ToolbarOptionsMenuName);
		}

		PopulateToolbarDropdownMenu(ToolbarOptionsMenu);
	}

	void RunExport()
	{
		const FQuikkProjectLogExportResult Result = UQuikkProjectLogLibrary::ExportCurrentProjectLog();

		FNotificationInfo NotificationInfo(FText::FromString(Result.Message));
		NotificationInfo.ExpireDuration = Result.bSuccess ? 6.0f : 10.0f;
		NotificationInfo.bFireAndForget = true;

		if (Result.bSuccess)
		{
			NotificationInfo.Hyperlink = FSimpleDelegate::CreateLambda([ExportedLogPath = Result.ExportedLogPath]()
			{
				FPlatformProcess::ExploreFolder(*FPaths::GetPath(ExportedLogPath));
			});
			NotificationInfo.HyperlinkText = FText::FromString(TEXT("Open Export Folder"));

			UE_LOG(LogTemp, Log, TEXT("%s"), *Result.Message);
		}
		else
		{
			NotificationInfo.bUseSuccessFailIcons = true;
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result.Message);
		}

		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}

	IConsoleObject* ExportCommand = nullptr;
	TSharedPtr<FUICommandList> PluginCommands;
	TSharedPtr<FSlateStyleSet> StyleSet;
	TSharedPtr<FExtender> ToolbarExtender;
};

IMPLEMENT_MODULE(FQuikkToolsEditorModule, QuikkToolsEditor)
