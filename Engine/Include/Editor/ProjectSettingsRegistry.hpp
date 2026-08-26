#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"

class EditorLayer;

class TE_API TEProjectSettings
{
public:
    virtual ~TEProjectSettings() = default;
    virtual TEString GetCategory() const = 0;
    virtual TEString GetDisplayName() const = 0;
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) = 0;
};

class TE_API ProjectSettingsRegistry
{
public:
    static void Register(TERef<TEProjectSettings> settings);
    static const TEArray<TERef<TEProjectSettings>> &GetSettings();
    static void Clear();

private:
    static ProjectSettingsRegistry &Instance();
    TEArray<TERef<TEProjectSettings>> m_ProjectSettings;
};

template <typename T> struct ProjectSettingsRegisterer
{
    ProjectSettingsRegisterer() { ProjectSettingsRegistry::Register(CreateRef<T>()); }
};

#define TE_REGISTER_PROJECT_SETTINGS(Type) static ProjectSettingsRegisterer<Type> Type##_ProjectSettingsReg;
