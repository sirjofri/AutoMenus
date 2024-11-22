// Copyright sirjofri. Licensed under MIT license. See License.txt for full license text.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AutoMenusSettings.generated.h"

USTRUCT()
struct FAutoMenuConfig
{
	GENERATED_BODY()

public:
	/**
	 * This is the visual name of the menu.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config")
	FName MenuName;

	/**
	 * This is the location of the menu.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config")
	FName MenuLocation = "MainFrame.MainMenu";

	/**
	 * Path to the widget blueprints.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", meta=(ContentDir, RelativeToGameContentDir))
	FDirectoryPath MenuEntryPath;

	/**
	 * Mapping of folders (submenus) to section names. Key must be a path relative to Menu Entry Path.
	 *
	 * Use this to map an automatically generated submenu to a specific menu section.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", AdvancedDisplay)
	TMap<FString,FName> FolderSectionMapping;

	/**
	 * Menu section where the menu will be added.
	 *
	 * This is important if the menu is a submenu of another menu, for example, a submenu within the "Tools" menu.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", AdvancedDisplay, AdvancedDisplay)
	FName MenuSection = NAME_None;

	/**
	 * Use this to specify where exactly to place the menu inside its parent menu.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", AdvancedDisplay, AdvancedDisplay)
	FName InsertLocation = NAME_None;

	/**
	 * Location relative to the Insert Location.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Auto Menu Config", AdvancedDisplay, AdvancedDisplay)
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
