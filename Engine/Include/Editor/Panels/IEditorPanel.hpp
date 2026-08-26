#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Layers/Layer.hpp"

class EditorLayer;

class TE_API IEditorPanel : public Layer
{
public:
    IEditorPanel(const TEString &name = "EditorPanel") : Layer(name) {}
    virtual ~IEditorPanel() = default;

    virtual TEString GetID() const = 0;
    virtual TEString GetTitle() const { return m_DebugName; }
    virtual bool IsVisible() const { return m_Visible; }
    virtual void SetVisible(bool visible) { m_Visible = visible; }
    virtual bool IsWindowMenuExposed() const { return true; }

    virtual void OnTimeGUIRender(Ref<EditorLayer> editor) {}
    virtual void OnTimeGUIRender() override {}

    // Shortcut listener hook for panel-specific handling
    virtual bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) { return false; }

protected:
    bool m_Visible = true;
};

class TE_API EditorPanelRegistry
{
public:
    static void RegisterPanel(TERef<IEditorPanel> panel);
    static TEArray<TERef<IEditorPanel>> GetPanels();
    static void Clear();
};

template <typename T> struct EditorPanelRegisterer
{
    EditorPanelRegisterer() { EditorPanelRegistry::RegisterPanel(CreateRef<T>()); }
};

#define TE_REGISTER_EDITOR_PANEL(Type) inline EditorPanelRegisterer<Type> Type##_EditorPanelReg;
