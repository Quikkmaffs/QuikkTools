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
	bool& bExists,
	int64& FileSize,
	bool& bCanOpenRead,
	bool& bCanReadBytes,
	FString& Diagnostic)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	bExists = PlatformFile.FileExists(*FullPath);
	FileSize = bExists ? PlatformFile.FileSize(*FullPath) : INDEX_NONE;
	bCanOpenRead = false;
	bCanReadBytes = false;

	if (bExists)
	{
		TUniquePtr<IFileHandle> FileHandle(PlatformFile.OpenRead(*FullPath));
		bCanOpenRead = FileHandle.IsValid();

		if (bCanOpenRead)
		{
			uint8 HeaderBytes[16] = {};
			const int64 BytesToRead = FMath::Min<int64>(UE_ARRAY_COUNT(HeaderBytes), FileSize);
			bCanReadBytes = BytesToRead > 0 ? FileHandle->Read(HeaderBytes, BytesToRead) : true;
		}
	}

	Diagnostic = FString::Printf(
		TEXT("path=%s exists=%d size=%lld canOpenRead=%d canReadBytes=%d"),
		*FullPath,
		bExists ? 1 : 0,
		FileSize,
		bCanOpenRead ? 1 : 0,
		bCanReadBytes ? 1 : 0);

	UE_LOG(LogQuikkFileAccess, Warning, TEXT("File access test: %s"), *Diagnostic);
	return bExists && bCanOpenRead;
}

bool UQuikkFileAccessLibrary::TestFileAccessInFolder(
	const FString& BaseFolderPath,
	const FString& FileName,
	FString& FullPath,
	bool& bExists,
	int64& FileSize,
	bool& bCanOpenRead,
	bool& bCanReadBytes,
	FString& Diagnostic)
{
	FullPath = BuildFilePathFromFolder(BaseFolderPath, FileName);
	return TestFileAccess(FullPath, bExists, FileSize, bCanOpenRead, bCanReadBytes, Diagnostic);
}
