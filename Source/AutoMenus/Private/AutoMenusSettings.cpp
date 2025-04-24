// Copyright sirjofri. See License.txt for full license text.


#include "AutoMenusSettings.h"

#include "ToolMenu.h"
#include "ToolMenus.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Framework/MultiBox/MultiBoxDefs.h"

void UAutoMenusSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	auto IsToolbar = [](EMultiBoxType Type)
	{
		return Type == EMultiBoxType::ToolBar
		|| Type == EMultiBoxType::UniformToolBar
		|| Type == EMultiBoxType::VerticalToolBar
		|| Type == EMultiBoxType::SlimHorizontalToolBar
#if (ENGINE_MAJOR_VERSION == 5) && (ENGINE_MINOR_VERSION >= 3)
		|| Type == EMultiBoxType::SlimHorizontalUniformToolBar
#endif
		;
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
