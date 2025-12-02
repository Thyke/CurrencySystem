// Copyright (C) Thyke. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FCurrencySystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
