#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuikkFileAccessLibrary.generated.h"

UCLASS()
class QUIKKTOOLSRUNTIME_API UQuikkFileAccessLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintPure,
		Category = "Quikk Tools|Storage",
		meta = (
			DisplayName = "Build File Path From Folder",
			ShortToolTip = "Build a full file path from a folder and file name.",
			ToolTip = "Combines Base Folder Path and FileName into a full file path.\n\nCommon Base Folder Path examples:\nAndroid OBB: /storage/emulated/0/Android/obb/com.genericapp.vr/\nAndroid App Files: /storage/emulated/0/Android/data/com.genericapp.vr/files/\nWindows: C:/Projects/MyApp/Content/Movies/\n\nLeave FileName empty to return the base folder path.",
			CPP_Default_BaseFolderPath = "/storage/emulated/0/Android/obb/com.genericapp.vr/"
		)
	)
	static FString BuildFilePathFromFolder(const FString& BaseFolderPath, const FString& FileName);

	UFUNCTION(
		BlueprintCallable,
		Category = "Quikk Tools|Storage",
		meta = (
			DisplayName = "Test File Access",
			ShortToolTip = "Test whether a full path exists and can be read.",
			ToolTip = "Tests whether Full Path exists, can be opened for reading, and can read the first bytes.\n\nUse this node when you already have a complete absolute file path."
		)
	)
	static bool TestFileAccess(
		const FString& FullPath,
		bool& bExists,
		int64& FileSize,
		bool& bCanOpenRead,
		bool& bCanReadBytes,
		FString& Diagnostic);

	UFUNCTION(
		BlueprintCallable,
		Category = "Quikk Tools|Storage",
		meta = (
			DisplayName = "Test File Access In Folder",
			ShortToolTip = "Build a file path from a folder and file name, then test access.",
			ToolTip = "Builds a full path from Base Folder Path and FileName, then tests whether the file exists and can be read.\n\nCommon Base Folder Path examples:\nAndroid OBB: /storage/emulated/0/Android/obb/com.genericapp.vr/\nAndroid App Files: /storage/emulated/0/Android/data/com.genericapp.vr/files/\nWindows: C:/Projects/MyApp/Content/Movies/\n\nPass only the file name in FileName. If you already have a complete absolute path, use Test File Access.",
			CPP_Default_BaseFolderPath = "/storage/emulated/0/Android/obb/com.genericapp.vr/",
			Keywords = "obb storage access android windows file path"
		)
	)
	static bool TestFileAccessInFolder(
		const FString& BaseFolderPath,
		const FString& FileName,
		FString& FullPath,
		bool& bExists,
		int64& FileSize,
		bool& bCanOpenRead,
		bool& bCanReadBytes,
		FString& Diagnostic);
};
