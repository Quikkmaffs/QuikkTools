#include "QuikkFileAccessLibrary.h"

#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuikkFileAccess, Log, All);

FString UQuikkFileAccessLibrary::BuildFilePathFromFolder(const FString& BaseFolderPath, const FString& FileName)
{
	if (BaseFolderPath.IsEmpty())
	{
		return FileName;
	}

	return FileName.IsEmpty() ? BaseFolderPath : FPaths::Combine(BaseFolderPath, FileName);
}

bool UQuikkFileAccessLibrary::TestFileAccess(
	const FString& FullPath,
	bool& Exists,
	int64& FileSize,
	bool& CanOpenRead,
	bool& CanReadBytes,
	FString& Diagnostic)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	Exists = PlatformFile.FileExists(*FullPath);
	FileSize = Exists ? PlatformFile.FileSize(*FullPath) : INDEX_NONE;
	CanOpenRead = false;
	CanReadBytes = false;

	if (Exists)
	{
		TUniquePtr<IFileHandle> FileHandle(PlatformFile.OpenRead(*FullPath));
		CanOpenRead = FileHandle.IsValid();

		if (CanOpenRead)
		{
			uint8 HeaderBytes[16] = {};
			const int64 BytesToRead = FMath::Min<int64>(UE_ARRAY_COUNT(HeaderBytes), FileSize);
			CanReadBytes = BytesToRead > 0 ? FileHandle->Read(HeaderBytes, BytesToRead) : true;
		}
	}

	Diagnostic = FString::Printf(
		TEXT("path=%s exists=%d size=%lld canOpenRead=%d canReadBytes=%d"),
		*FullPath,
		Exists ? 1 : 0,
		FileSize,
		CanOpenRead ? 1 : 0,
		CanReadBytes ? 1 : 0);

	UE_LOG(LogQuikkFileAccess, Warning, TEXT("File access test: %s"), *Diagnostic);
	return Exists && CanOpenRead;
}

void UQuikkFileAccessLibrary::ResolveFilePath(
	const FString& BaseFolderPath,
	const FString& FileName,
	FString& FullPath,
	bool& Exists,
	bool& Accessible,
	int64& FileSize,
	bool& CanOpenRead,
	bool& CanReadBytes,
	FString& Diagnostic)
{
	FullPath = BuildFilePathFromFolder(BaseFolderPath, FileName);
	Accessible = TestFileAccess(FullPath, Exists, FileSize, CanOpenRead, CanReadBytes, Diagnostic);
}
