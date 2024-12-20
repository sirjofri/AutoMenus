﻿// Copyright sirjofri. Licensed under MIT license. See License.txt for full license text.

#pragma once

#include "CoreMinimal.h"
#include "AutoMenusSettings.h"
#include "Modules/ModuleManager.h"

class UEditorUtilityWidgetBlueprint;
class UWidgetBlueprint;
class UEditorUtilityWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogAutoMenus, Log, All);

class FAutoMenusModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void GenerateMenu(UToolMenu* ToolMenu, FString Path, FAutoMenuConfig Config);
	static void OnObjectPreSave(UObject* Object, FObjectPreSaveContext ObjectPreSaveContext);

	void ReloadTopLevelMenus(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);
	void RemoveTopLevelMenus();
	void GenerateTopLevelMenus();

	static bool MenuTypeNeedsWidget(EMultiBoxType Type);

	TSharedRef<SWidget> MakeToolBarWidget(FName MenuName, const FAutoMenuConfig& Conf);
	TSharedRef<SWidget> MakeToolBarMenuContent(FName MenuName, FAutoMenuConfig Conf);

private:
	static void StartEditorWidget(UWidgetBlueprint* WidgetBlueprint);
	static void AddEntryToMenu(UToolMenu* ToolMenu, FAssetData AssetData);
	static TArray<FAssetData> GetMenuEntries(const FString& Folder);

	static FString GetWidgetTitle(const UEditorUtilityWidgetBlueprint* WidgetBlueprint);
	static FString GetTooltipText(const UEditorUtilityWidgetBlueprint* WidgetBlueprint);
	static FString GetMenuSection(const UEditorUtilityWidgetBlueprint* WidgetBlueprint);

	static const UAutoMenusSettings* GetConfig();

	TMap<FName,FAutoMenuConfig> GeneratedMenus;
};
