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
		BlueprintCallable,
		Category = "Quikk Tools|Storage",
		meta = (
			DisplayName = "Resolve File Path",
			ShortToolTip = "Resolve a file path from a folder and file name, then inspect whether it is accessible.",
			ToolTip = "Builds a full path from Base Folder Path and FileName, then checks whether the resolved file exists and can be read.\n\nCommon Base Folder Path examples:\nAndroid OBB: /storage/emulated/0/Android/obb/com.genericapp.vr/\nAndroid App Files: /storage/emulated/0/Android/data/com.genericapp.vr/files/\nWindows: C:/Projects/MyApp/Content/Movies/\n\nPass only the file name in FileName. Use the advanced outputs when you need more detailed diagnostics.",
			CPP_Default_BaseFolderPath = "/storage/emulated/0/Android/obb/com.genericapp.vr/",
			Keywords = "resolve file path storage access android windows file path",
			AdvancedDisplay = "Accessible,FileSize,CanOpenRead,CanReadBytes,Diagnostic"
		)
	)
	static void ResolveFilePath(
		const FString& BaseFolderPath,
		const FString& FileName,
		FString& FullPath,
		bool& Exists,
		bool& Accessible,
		int64& FileSize,
		bool& CanOpenRead,
		bool& CanReadBytes,
		FString& Diagnostic);

	static FString BuildFilePathFromFolder(const FString& BaseFolderPath, const FString& FileName);

	static bool TestFileAccess(
		const FString& FullPath,
		bool& Exists,
		int64& FileSize,
		bool& CanOpenRead,
		bool& CanReadBytes,
		FString& Diagnostic);
};
