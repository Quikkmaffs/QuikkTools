#include "QuikkProjectLogLibrary.h"

#include "Framework/Notifications/NotificationManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"

class FQuikkToolsEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
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
	}

private:
	void AddExportLogMenuEntry(FToolMenuSection& Section, const FToolUIActionChoice& ExportAction) const
	{
		Section.AddMenuEntry(
			TEXT("QuikkTools.ExportCurrentProjectLog"),
			FText::FromString(TEXT("Export Current Project Log")),
			FText::FromString(TEXT("Copy Saved/Logs/<Project>.log into the project's Exports folder.")),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Save")),
			ExportAction
		);
	}

	void BuildToolbarDropdownMenu(UToolMenu* InMenu)
	{
		if (InMenu->FindEntry(TEXT("QuikkTools.ExportCurrentProjectLog")) != nullptr)
		{
			return;
		}

		const FToolUIActionChoice ExportAction(FExecuteAction::CreateRaw(this, &FQuikkToolsEditorModule::RunExport));

		FToolMenuSection& ToolsSection = InMenu->FindOrAddSection(TEXT("QuikkToolsAvailable"));
		ToolsSection.Label = FText::FromString(TEXT("Available Tools"));
		AddExportLogMenuEntry(ToolsSection, ExportAction);
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
			FText::FromString(TEXT("Open the QuikkTools menu to access editor utilities.")),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Save"))
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
};

IMPLEMENT_MODULE(FQuikkToolsEditorModule, QuikkToolsEditor)
