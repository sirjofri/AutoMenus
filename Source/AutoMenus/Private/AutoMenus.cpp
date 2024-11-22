// Copyright sirjofri. Licensed under MIT license. See License.txt for full license text.

#include "AutoMenus.h"

#include "Editor.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Engine/AssetManager.h"

DEFINE_LOG_CATEGORY(LogAutoMenus);

#define LOCTEXT_NAMESPACE "FAutoMenusModule"

inline static const FName NAME_ToolTipProperty(TEXT("ToolTipText"));
inline static const FName NAME_MenuSectionProperty(TEXT("MenuSection"));

inline static const FName NAME_MenuSectionTag(TEXT("AutoMenus.MenuSection"));
inline static const FName NAME_TabNameTag(TEXT("AutoMenus.TabName"));
inline static const FName NAME_ToolTipTag(TEXT("AutoMenus.ToolTip"));

void FAutoMenusModule::StartupModule()
{
	FCoreUObjectDelegates::OnObjectPreSave.AddStatic(&FAutoMenusModule::OnObjectPreSave);

	TSet<FName>& MetaDataTagsForAssetRegistry = UObject::GetMetaDataTagsForAssetRegistry();
	MetaDataTagsForAssetRegistry.Add(NAME_MenuSectionTag);
	MetaDataTagsForAssetRegistry.Add(NAME_TabNameTag);
	MetaDataTagsForAssetRegistry.Add(NAME_ToolTipTag);

	GetMutableDefault<UAutoMenusSettings>()->OnSettingChanged().AddRaw(this, &FAutoMenusModule::ReloadTopLevelMenus);
	GenerateTopLevelMenus();
}

void FAutoMenusModule::ShutdownModule()
{
	RemoveTopLevelMenus();
}

void FAutoMenusModule::GenerateMenu(UToolMenu* ToolMenu, FString Path, FAutoMenuConfig Config)
{
	FString folder = Config.MenuEntryPath.Path / Path;
	folder.RemoveFromEnd(TEXT("/"));

	TArray<FAssetData> entries = GetMenuEntries(folder);

	TArray<FString> Subfolders;
	
	for (FAssetData entry : entries) {
		FString s = entry.PackageName.ToString();
		s.RemoveFromStart(folder / TEXT(""));

		TArray<FString> out;
		s.ParseIntoArray(out, TEXT("/"));
		
		if (out.Num() == 1) {
			// one hierarchy: item is within folder
			
			UE_LOG(LogAutoMenus, Verbose, TEXT("Found Entry:  Add  %s"), *s);
			AddEntryToMenu(ToolMenu, entry);
			
		} else if (out.Num() == 2) {
			// two hierarchies: item is in subfolder

			FString SubPath = FPaths::GetPath(s);
			FString SubName = FPaths::GetBaseFilename(SubPath);
			FString FullPath = Path / SubPath;
			UE_LOG(LogAutoMenus, Verbose, TEXT("Found Folder: Menu %s"), *FullPath);

			// we already created this submenu, skip creating submenu
			if (Subfolders.Contains(SubPath))
				continue;

			Subfolders.AddUnique(SubPath);

			FName SectionName = NAME_None;
			{
				if (FName *Found = Config.FolderSectionMapping.Find(FullPath))
					SectionName = *Found;
			}

			FToolMenuSection& ToolMenuSection = ToolMenu->FindOrAddSection(SectionName);
			if (!SectionName.IsNone())
				ToolMenuSection.Label = FText::FromName(SectionName);
			
			ToolMenuSection.AddSubMenu(FName(SubName), FText::FromString(SubName), FText::GetEmpty(), FNewToolMenuDelegate::CreateRaw(this, &FAutoMenusModule::GenerateMenu, FullPath, Config));
			
		} else {
			// more than two hierarchies: item is in a deeper hierarchy, ignore
			continue;
		}
	}
}

void FAutoMenusModule::OnObjectPreSave(UObject* Object, FObjectPreSaveContext ObjectPreSaveContext)
{
	const UEditorUtilityWidgetBlueprint* WidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(Object);
	if (!WidgetBlueprint)
		return;

	UMetaData* metadata = WidgetBlueprint->GetPackage()->GetMetaData();
	
	FString title = GetWidgetTitle(WidgetBlueprint);
	if (title.IsEmpty()) {
		// if we have no dedicated tab name, assume this is a helper widget (button, ...)
		metadata->RemoveValue(WidgetBlueprint, NAME_TabNameTag);
		metadata->RemoveValue(WidgetBlueprint, NAME_ToolTipTag);
		metadata->RemoveValue(WidgetBlueprint, NAME_MenuSectionTag);
		return;
	}

	metadata->SetValue(WidgetBlueprint, NAME_TabNameTag, *title);
	metadata->SetValue(WidgetBlueprint, NAME_ToolTipTag, *GetTooltipText(WidgetBlueprint));
	metadata->SetValue(WidgetBlueprint, NAME_MenuSectionTag, *GetMenuSection(WidgetBlueprint));
}

void FAutoMenusModule::ReloadTopLevelMenus(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	RemoveTopLevelMenus();
	GeneratedMenus.Reserve(GetConfig()->MenuConfig.Num());
	GenerateTopLevelMenus();
}

void FAutoMenusModule::RemoveTopLevelMenus()
{
	for (TTuple<FName,FAutoMenuConfig> kv : GeneratedMenus) {
		UToolMenus::Get()->RemoveMenu(kv.Key);
		UToolMenus::Get()->RemoveEntry(kv.Value.MenuLocation, kv.Value.MenuSection, kv.Key);
		
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(kv.Value.MenuLocation);
		FToolMenuSection* Section = Menu->FindSection(kv.Value.MenuSection);
		if (Section && Section->Blocks.IsEmpty())
			Menu->RemoveSection(kv.Value.MenuSection);
	}
	GeneratedMenus.Empty();
}

void FAutoMenusModule::GenerateTopLevelMenus()
{
	for (FAutoMenuConfig conf : GetConfig()->MenuConfig) {
		if (conf.MenuName.IsNone())
			continue;
		if (conf.MenuLocation.IsNone())
			continue;
		if (conf.MenuEntryPath.Path.IsEmpty())
			continue;

		FName MenuName = FName(TEXT("AutoMenus_") + conf.MenuName.ToString());
		GeneratedMenus.Add(MenuName, conf);
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(conf.MenuLocation);
		FToolMenuSection& sec = Menu->FindOrAddSection(conf.MenuSection);
		sec.Label = FText::FromName(conf.MenuSection);
		FToolMenuEntry& entry = sec.AddSubMenu(MenuName, FText::FromName(conf.MenuName), FText::GetEmpty(), FNewToolMenuDelegate::CreateRaw(this, &FAutoMenusModule::GenerateMenu, FString(TEXT("")), conf));
		entry.InsertPosition = FToolMenuInsert(conf.InsertLocation, conf.InsertType);
	}
	UToolMenus::Get()->RefreshAllWidgets();
}

void FAutoMenusModule::StartEditorWidget(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint)
		return;

	if (!WidgetBlueprint->GeneratedClass->IsChildOf(UEditorUtilityWidget::StaticClass()))
		return;

	UEditorUtilityWidgetBlueprint* Widget = Cast<UEditorUtilityWidgetBlueprint>(WidgetBlueprint);
	if (!Widget) {
		UE_LOG(LogAutoMenus, Warning, TEXT("Invalid Widget Blueprint"));
		return;
	}
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	EditorUtilitySubsystem->SpawnAndRegisterTab(Widget);
}

void FAutoMenusModule::AddEntryToMenu(UToolMenu* ToolMenu, FAssetData AssetData)
{
	FString AssetPath = AssetData.PackageName.ToString();

	FString title;
	// if we don't have a tab name, ignore this script. It's probably a helper widget
	if (!AssetData.GetTagValue(NAME_TabNameTag, title))
		return;

	FName MenuSection = NAME_None;
	AssetData.GetTagValue(NAME_MenuSectionTag, MenuSection);
	
	FToolMenuSection& sec = ToolMenu->FindOrAddSection(MenuSection);
	if (!MenuSection.IsNone())
		sec.Label = FText::FromName(MenuSection);

	FString tooltip;
	AssetData.GetTagValue(NAME_ToolTipTag, tooltip);

	static const FText EditTextPrompt = LOCTEXT("ToolTipEditTextPrompt", "Shift-click to edit script");
	static const FText EditToolTipPrompt = LOCTEXT("ToolTipEditToolTipPrompt", "(Set values in asset)");
	FText ToolTipContent = EditToolTipPrompt;
	if (!tooltip.IsEmpty())
		ToolTipContent = FText::FromString(tooltip);
	FText ToolTipText = FText::Format(INVTEXT("{0}\n\n({1})"), ToolTipContent, EditTextPrompt);
	
	sec.AddMenuEntry(FName(title), FText::FromString(title), ToolTipText, FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([&](FString InAssetPath)
		{
			const TSoftObjectPtr<UObject> LocalObjPtr(InAssetPath + TEXT(".") + FPaths::GetBaseFilename(InAssetPath));
			UObject* o = LocalObjPtr.LoadSynchronous();
			if (FSlateApplication::Get().GetModifierKeys().IsShiftDown()) {
				GEditor->EditObject(o);
				return;
			}
			UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(o);
			if (!WidgetBlueprint)
				return;
			StartEditorWidget(WidgetBlueprint);
		}, AssetPath)));
}

TArray<FAssetData> FAutoMenusModule::GetMenuEntries(const FString& Folder)
{
	TArray<FAssetData> Ret;

	const IAssetRegistry& AssetRegistry = UAssetManager::Get().GetAssetRegistry();
	FARFilter Filter;
	Filter.PackagePaths.Add(FName(Folder));
	Filter.ClassPaths.Add(UEditorUtilityWidgetBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	TArray<FAssetData> AssetData;
	AssetRegistry.GetAssets(Filter, AssetData);

	for (FAssetData a : AssetData) {
		if (a.AssetClassPath != UEditorUtilityWidgetBlueprint::StaticClass()->GetClassPathName())
			continue;
		
		Ret.Add(a);
	}

	return Ret;
}

FString FAutoMenusModule::GetWidgetTitle(const UEditorUtilityWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint)
		return TEXT("");
	UEditorUtilityWidget* Widget = WidgetBlueprint->GeneratedClass->GetDefaultObject<UEditorUtilityWidget>();
	return Widget->GetTabDisplayName().ToString();
}

FString FAutoMenusModule::GetTooltipText(const UEditorUtilityWidgetBlueprint* WidgetBlueprint)
{
	UEditorUtilityWidget* widget = WidgetBlueprint->GeneratedClass->GetDefaultObject<UEditorUtilityWidget>();
	if (!widget)
		return TEXT("");

	const FTextProperty* ToolTipProperty = CastField<FTextProperty>(widget->StaticClass()->FindPropertyByName(NAME_ToolTipProperty));
	if (!ToolTipProperty)
		return TEXT("");

	const FText* ToolTipText = ToolTipProperty->ContainerPtrToValuePtr<FText>(widget);
	return ToolTipText->ToString();
}

FString FAutoMenusModule::GetMenuSection(const UEditorUtilityWidgetBlueprint* WidgetBlueprint)
{
	UEditorUtilityWidget* Widget = WidgetBlueprint->GeneratedClass->GetDefaultObject<UEditorUtilityWidget>();
	if (!Widget)
		return TEXT("");

	FName SectionName = NAME_None;
	for (TFieldIterator<FNameProperty> It(Widget->GetClass()); It; ++It) {
		if (It->GetFName() == NAME_MenuSectionProperty) {
			SectionName = It->GetPropertyValue(It->ContainerPtrToValuePtr<FName>(Widget));
			break;
		}
	}
	return SectionName.ToString();
}

const UAutoMenusSettings* FAutoMenusModule::GetConfig()
{
	return GetDefault<UAutoMenusSettings>();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAutoMenusModule, AutoMenus)