#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <functional>



class EditorLayer;

class TE_API EditorMode
{
public:
    virtual ~EditorMode() = default;
    virtual void OnEnter() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnTimeGUIRender() {}
    virtual void OnExit() {}
    virtual bool OnShortcut(const TEString &shortcutId) { return false; }
    virtual TEString GetName() const = 0;
    virtual TEString GetIcon() const { return ""; }

    // Panel & Layout Policy
    virtual bool ShouldHideStandardPanels() const { return false; }
    virtual bool IsPanelAllowed(const TEString &panelId) const { return !ShouldHideStandardPanels(); }
    virtual bool WantsFullscreenWorkspace() const { return false; }
    virtual TEString GetWorkspaceWindowName() const { return GetName() + " Studio"; }
    virtual void RegisterMenubarItems(Ref<EditorLayer> editor) {}
};

class TE_API EditorModeRegistry
{
public:
    EditorModeRegistry(const EditorModeRegistry &) = delete;
    EditorModeRegistry &operator=(const EditorModeRegistry &) = delete;
    EditorModeRegistry(EditorModeRegistry &&) = delete;
    EditorModeRegistry &operator=(EditorModeRegistry &&) = delete;

    template <typename T, typename... Args> static void RegisterMode(Args &&...args)
    {
        RegisterModeInternal(CreateScope<T>(std::forward<Args>(args)...));
    }

    static void RegisterModeInternal(TEScope<EditorMode> mode);
    static void UnregisterMode(const TEString &name);
    static void SetActiveMode(const TEString &name);
    static EditorMode *GetActiveMode();
    static const TEArray<TEScope<EditorMode>> &GetModes();
    static void Clear();

private:
    EditorModeRegistry() = default;
    ~EditorModeRegistry() = default;
    static EditorModeRegistry &Instance();

    TEArray<TEScope<EditorMode>> m_Modes;
    EditorMode *m_ActiveMode = nullptr;
};

// Auto-registration helper
template <typename T> struct EditorModeRegisterer
{
    EditorModeRegisterer() { EditorModeRegistry::RegisterMode<T>(); }
};

#define T_REGISTER_EDITOR_MODE(Type) static EditorModeRegisterer<Type> Type##_Registerer;

