#pragma once
#include "PreRequisites.h"
#include "GameplayUtils.hpp"

class EditorLayer;

class TE_API TEProjectSettings
{
public:
    virtual ~TEProjectSettings() = default;
    virtual TEString GetCategory() const = 0;
    virtual TEString GetDisplayName() const = 0;
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) = 0;
    virtual bool RequiresRestart() const { return m_RequiresRestart; }
    void SetRequiresRestart(bool requiresRestart) { m_RequiresRestart = requiresRestart; }

private:
    bool m_RequiresRestart = false;
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
    ProjectSettingsRegisterer(bool requiresRestart = false)
    {
        auto inst = CreateRef<T>();
        inst->SetRequiresRestart(requiresRestart);
        ProjectSettingsRegistry::Register(inst);
    }
};

#define TE_REGISTER_PROJECT_SETTINGS(Type, ...)                                                                        \
    inline ProjectSettingsRegisterer<Type> Type##_ProjectSettingsReg{__VA_ARGS__};
