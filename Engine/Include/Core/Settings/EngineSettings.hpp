#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <typeindex>
/**
 * EngineSettings - Base developer settings interface
 * Derive from this class and register with TE_REGISTER_SETTINGS(MySettingsClass)
 */
class TE_API EngineSettings
{
public:
    EngineSettings() = default;
    virtual ~EngineSettings() = default;

    virtual TEString GetCategoryName() const { return "General"; }
    virtual TEString GetSectionName() const { return "Engine"; }
    virtual TEString GetDisplayName() const { return GetSectionName(); }
    virtual TEString GetDescription() const { return ""; }

    virtual void OnInitialized() {}
    virtual void OnModified() {}
    virtual void ResetToDefaults() {}
    virtual bool ValidateSettings() const { return true; }
    virtual TEString GetValidationErrors() const { return ""; }

    virtual bool Save(const TEString &configDir) { return true; }
    virtual bool Load(const TEString &configDir) { return true; }
};

