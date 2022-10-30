// Bartender Ender
// Naughty Panda @ 2022

#pragma once

#include "CoreMinimal.h"

class FGeometryToolsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
