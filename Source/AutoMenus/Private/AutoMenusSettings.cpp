// Copyright Lightword Productions GmbH. All rights reserved.


#include "AutoMenusSettings.h"

#include "ToolMenu.h"
#include "ToolMenus.h"
#include "Framework/MultiBox/MultiBoxDefs.h"

void UAutoMenusSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	auto IsToolbar = [](EMultiBoxType Type)
	{
		return Type == EMultiBoxType::ToolBar
		|| Type == EMultiBoxType::UniformToolBar
		|| Type == EMultiBoxType::VerticalToolBar
		|| Type == EMultiBoxType::SlimHorizontalToolBar
		|| Type == EMultiBoxType::SlimHorizontalUniformToolBar;
	};

	for (FAutoMenuConfig& conf : MenuConfig) {
		UToolMenu* Menu = UToolMenus::Get()->FindMenu(conf.MenuLocation);
		if (!Menu)
			continue;
		conf.IsToolbarMenu = IsToolbar(Menu->MenuType);
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

UAutoMenusSettings::UAutoMenusSettings()
{
	SectionName = FName("Auto Menus");
	CategoryName = FName("Plugins");
}
