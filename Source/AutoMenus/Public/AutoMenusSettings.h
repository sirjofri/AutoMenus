// Copyright Lightword Productions GmbH. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AutoMenusSettings.generated.h"

USTRUCT()
struct FAutoMenuConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", meta=(ConfigRestartRequired="true"))
	FName MenuName;

	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", meta=(ConfigRestartRequired="true"))
	FName MenuLocation = "MainFrame.MainMenu";

	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", meta=(ContentDir, RelativeToGameContentDir, ConfigRestartRequired="true"))
	FDirectoryPath MenuEntryPath;

	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", meta=(ConfigRestartRequired="true"))
	TMap<FString,FName> FolderSectionMapping;

	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", AdvancedDisplay, meta=(ConfigRestartRequired="true"))
	FName MenuSection = NAME_None;

	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", AdvancedDisplay, meta=(ConfigRestartRequired="true"))
	FName InsertLocation = NAME_None;

	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", AdvancedDisplay, meta=(ConfigRestartRequired="true"))
	EToolMenuInsertType InsertType = EToolMenuInsertType::Default;
};


UCLASS(Config=EditorPerProjectUserSettings, DefaultConfig, meta=(DisplayName="Auto Menus"))
class AUTOMENUS_API UAutoMenusSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category="Auto Menus", meta=(TitleProperty="MenuName"))
	TArray<FAutoMenuConfig> MenuConfig;

	UAutoMenusSettings();
};
