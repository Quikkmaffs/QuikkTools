#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuikkProjectLogLibrary.generated.h"

USTRUCT(BlueprintType)
struct FQuikkProjectLogExportResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString ProjectName;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString BuildTargetName;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString BuildTargetType;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString BuildConfiguration;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString TimestampLocal;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString SourceLogPath;

	UPROPERTY(BlueprintReadOnly, Category = "Quikk Tools|Project Logs")
	FString ExportedLogPath;
};

UCLASS()
class QUIKKTOOLSEDITOR_API UQuikkProjectLogLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quikk Tools|Project Logs")
	static FQuikkProjectLogExportResult ExportCurrentProjectLog();
};
