#include "QuikkProjectLogLibrary.h"

#include "Brushes/SlateImageBrush.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"

class FQuikkToolsEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		RegisterStyle();

		ExportCommand = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("QuikkTools.ExportCurrentProjectLog"),
			TEXT("Copies Saved/Logs/<Project>.log into the project's Exports folder."),
			FConsoleCommandDelegate::CreateRaw(this, &FQuikkToolsEditorModule::RunExport),
			ECVF_Default
		);

		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FQuikkToolsEditorModule::RegisterMenus)
		);
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
		NewStyleSet->SetContentRoot(FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources")));
		NewStyleSet->Set(
			TEXT("QuikkTools.ToolbarIcon"),
			new FSlateVectorImageBrush(NewStyleSet->RootToContentDir(TEXT("QuikkToolsIcon"), TEXT(".svg")), FVector2D(20.0f, 20.0f))
		);
		NewStyleSet->Set(
			TEXT("QuikkTools.MenuIcon"),
			new FSlateVectorImageBrush(NewStyleSet->RootToContentDir(TEXT("QuikkToolsIcon"), TEXT(".svg")), FVector2D(16.0f, 16.0f))
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

	void BuildToolbarDropdownMenu(UToolMenu* InMenu)
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

	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);
		const FToolUIActionChoice ExportAction(FExecuteAction::CreateRaw(this, &FQuikkToolsEditorModule::RunExport));

		UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
		FToolMenuSection& Section = ToolsMenu->FindOrAddSection(TEXT("QuikkTools"));
		AddExportLogMenuEntry(Section, ExportAction);

		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar"));
		FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection(TEXT("QuikkTools"));
		ToolbarSection.InsertPosition = FToolMenuInsert(TEXT("Play"), EToolMenuInsertType::After);
		FToolMenuEntry ToolbarEntry = FToolMenuEntry::InitComboButton(
			TEXT("QuikkTools.ToolbarMenu"),
			FToolUIActionChoice(),
			FNewToolMenuChoice(FNewToolMenuDelegate::CreateRaw(this, &FQuikkToolsEditorModule::BuildToolbarDropdownMenu)),
			FText::FromString(TEXT("QuikkTools")),
			FText::FromString(TEXT("Open QuikkTools to access logs, diagnostics, and utilities.")),
			GetQuikkToolsToolbarIcon()
		);
		ToolbarEntry.StyleNameOverride = TEXT("CalloutToolbar");
		ToolbarSection.AddEntry(ToolbarEntry);
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
	TSharedPtr<FSlateStyleSet> StyleSet;
};

IMPLEMENT_MODULE(FQuikkToolsEditorModule, QuikkToolsEditor)
