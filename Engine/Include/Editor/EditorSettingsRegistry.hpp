#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"

class EditorLayer;

class TE_API TEEditorSettings
{
public:
    virtual ~TEEditorSettings() = default;
    virtual TEString GetCategory() const = 0;
    virtual TEString GetDisplayName() const = 0;
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) = 0;
};

class TE_API EditorSettingsRegistry
{
public:
    static void Register(TERef<TEEditorSettings> settings);
    static const TEArray<TERef<TEEditorSettings>> &GetSettings();
    static void Clear();

private:
    static EditorSettingsRegistry &Instance();
    TEArray<TERef<TEEditorSettings>> m_EditorSettings;
};

template <typename T> struct EditorSettingsRegisterer
{
    EditorSettingsRegisterer() { EditorSettingsRegistry::Register(CreateRef<T>()); }
};

#define TE_REGISTER_EDITOR_SETTINGS(Type) static EditorSettingsRegisterer<Type> Type##_EditorSettingsReg;
