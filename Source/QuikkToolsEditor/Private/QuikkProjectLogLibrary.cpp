#include "QuikkProjectLogLibrary.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace QuikkProjectLog
{
	FString GetProjectName()
	{
		FString ProjectName = FApp::GetProjectName();
		if (ProjectName.IsEmpty())
		{
			ProjectName = FPaths::GetBaseFilename(FPaths::GetProjectFilePath());
		}

		return ProjectName.IsEmpty() ? TEXT("UnknownProject") : ProjectName;
	}

	FString GetBuildConfigurationString()
	{
		switch (FApp::GetBuildConfiguration())
		{
		case EBuildConfiguration::Debug:
			return TEXT("Debug");
		case EBuildConfiguration::DebugGame:
			return TEXT("DebugGame");
		case EBuildConfiguration::Development:
			return TEXT("Development");
		case EBuildConfiguration::Shipping:
			return TEXT("Shipping");
		case EBuildConfiguration::Test:
			return TEXT("Test");
		default:
			return TEXT("UnknownConfig");
		}
	}

	FString GetCompiledTargetTypeString()
	{
#if WITH_EDITOR
		return TEXT("Editor");
#elif UE_SERVER
		return TEXT("Server");
#elif UE_CLIENT
		return TEXT("Client");
#else
		return TEXT("Game");
#endif
	}

	FString MakeFilenameToken(const FString& Value, const FString& Fallback)
	{
		FString Token = Value.IsEmpty() ? Fallback : Value;
		Token.ReplaceInline(TEXT(" "), TEXT("_"));
		Token = FPaths::MakeValidFileName(Token, '_');
		return Token.IsEmpty() ? Fallback : Token;
	}

	FString ReadTargetNameFromTargetInfo(const FString& ProjectName, const FString& BuildTargetType)
	{
		const FString TargetInfoPath = FPaths::Combine(FPaths::ProjectIntermediateDir(), TEXT("TargetInfo.json"));
		FString TargetInfoJson;

		if (!FFileHelper::LoadFileToString(TargetInfoJson, *TargetInfoPath))
		{
			return FString();
		}

		TSharedPtr<FJsonObject> RootObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(TargetInfoJson);
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			return FString();
		}

		const TArray<TSharedPtr<FJsonValue>>* Targets = nullptr;
		if (!RootObject->TryGetArrayField(TEXT("Targets"), Targets) || Targets == nullptr)
		{
			return FString();
		}

		for (const TSharedPtr<FJsonValue>& TargetValue : *Targets)
		{
			if (!TargetValue.IsValid())
			{
				continue;
			}

			const TSharedPtr<FJsonObject>* TargetObject = nullptr;
			if (!TargetValue->TryGetObject(TargetObject) || TargetObject == nullptr || !TargetObject->IsValid())
			{
				continue;
			}

			FString CandidateType;
			FString CandidateName;
			(*TargetObject)->TryGetStringField(TEXT("Type"), CandidateType);
			(*TargetObject)->TryGetStringField(TEXT("Name"), CandidateName);

			if (CandidateType.Equals(BuildTargetType, ESearchCase::IgnoreCase) && !CandidateName.IsEmpty())
			{
				return CandidateName;
			}
		}

		for (const TSharedPtr<FJsonValue>& TargetValue : *Targets)
		{
			if (!TargetValue.IsValid())
			{
				continue;
			}

			const TSharedPtr<FJsonObject>* TargetObject = nullptr;
			if (!TargetValue->TryGetObject(TargetObject) || TargetObject == nullptr || !TargetObject->IsValid())
			{
				continue;
			}

			FString CandidateName;
			(*TargetObject)->TryGetStringField(TEXT("Name"), CandidateName);
			if (CandidateName.StartsWith(ProjectName))
			{
				return CandidateName;
			}
		}

		return FString();
	}

	FString GetBuildTargetName(const FString& ProjectName, const FString& BuildTargetType)
	{
		const FString TargetName = ReadTargetNameFromTargetInfo(ProjectName, BuildTargetType);
		if (!TargetName.IsEmpty())
		{
			return TargetName;
		}

		if (BuildTargetType.Equals(TEXT("Editor"), ESearchCase::IgnoreCase))
		{
			return FString::Printf(TEXT("%sEditor"), *ProjectName);
		}

		if (BuildTargetType.Equals(TEXT("Game"), ESearchCase::IgnoreCase))
		{
			return ProjectName;
		}

		return BuildTargetType.IsEmpty() ? TEXT("UnknownTarget") : BuildTargetType;
	}
}

FQuikkProjectLogExportResult UQuikkProjectLogLibrary::ExportCurrentProjectLog()
{
	FQuikkProjectLogExportResult Result;

	Result.ProjectName = QuikkProjectLog::GetProjectName();
	Result.BuildTargetType = QuikkProjectLog::GetCompiledTargetTypeString();
	Result.BuildTargetName = QuikkProjectLog::GetBuildTargetName(Result.ProjectName, Result.BuildTargetType);
	Result.BuildConfiguration = QuikkProjectLog::GetBuildConfigurationString();

	const FDateTime Now = FDateTime::Now();
	Result.TimestampLocal = Now.ToString(TEXT("%Y-%m-%d %H:%M:%S"));

	Result.SourceLogPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Logs"),
		FString::Printf(TEXT("%s.log"), *Result.ProjectName)
	);
	Result.SourceLogPath = FPaths::ConvertRelativePathToFull(Result.SourceLogPath);

	const FString ExportDirectory = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectDir(), TEXT("Exports"))
	);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*ExportDirectory) && !PlatformFile.CreateDirectoryTree(*ExportDirectory))
	{
		Result.Message = FString::Printf(TEXT("Could not create export folder: %s"), *ExportDirectory);
		return Result;
	}

	if (!PlatformFile.FileExists(*Result.SourceLogPath))
	{
		Result.Message = FString::Printf(TEXT("Expected log file was not found: %s"), *Result.SourceLogPath);
		return Result;
	}

	const FString ExportFilename = FString::Printf(
		TEXT("%s__%s__%s__%s.log"),
		*QuikkProjectLog::MakeFilenameToken(Result.ProjectName, TEXT("Project")),
		*QuikkProjectLog::MakeFilenameToken(Result.BuildTargetName, TEXT("Target")),
		*QuikkProjectLog::MakeFilenameToken(Result.BuildConfiguration, TEXT("Config")),
		*Now.ToString(TEXT("%Y%m%d-%H%M%S"))
	);

	Result.ExportedLogPath = FPaths::Combine(ExportDirectory, ExportFilename);
	Result.ExportedLogPath = FPaths::ConvertRelativePathToFull(Result.ExportedLogPath);

	const uint32 CopyResult = IFileManager::Get().Copy(
		*Result.ExportedLogPath,
		*Result.SourceLogPath,
		true,
		false,
		false,
		nullptr,
		FILEREAD_AllowWrite,
		FILEWRITE_AllowRead
	);

	if (CopyResult != COPY_OK)
	{
		Result.Message = FString::Printf(
			TEXT("Could not copy log from %s to %s (CopyResult=%u)"),
			*Result.SourceLogPath,
			*Result.ExportedLogPath,
			CopyResult
		);
		return Result;
	}

	Result.bSuccess = true;
	Result.Message = FString::Printf(
		TEXT("Exported %s to %s"),
		*FPaths::GetCleanFilename(Result.SourceLogPath),
		*Result.ExportedLogPath
	);

	return Result;
}
