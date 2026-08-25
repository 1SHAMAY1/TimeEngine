#pragma once
#include "Core/PreRequisites.h"
#include "Editor/EditorToolbarRegistry.hpp"


class EditorLayer;

class TE_API IEditorToolbarOverlay
{
public:
    virtual ~IEditorToolbarOverlay() = default;
    virtual void RegisterToolbarItems(Ref<EditorLayer> editor) = 0;
    virtual void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) {}
    virtual void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) {}
    virtual bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) { return false; }
};

class TE_API EditorToolbarOverlayRegistry
{
public:
    static void RegisterOverlay(TERef<IEditorToolbarOverlay> overlay);
    static TEArray<TERef<IEditorToolbarOverlay>> GetOverlays();
    static void Clear();
};

template <typename T> struct EditorToolbarOverlayRegisterer
{
    EditorToolbarOverlayRegisterer() { EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<T>()); }
};

#define TE_REGISTER_TOOLBAR_OVERLAY(Type) static EditorToolbarOverlayRegisterer<Type> Type##_ToolbarOverlayReg;

class TE_API SaveAllToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
    bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) override;
    static void OpenSaveModal(bool isAppExit = false);
    static void OpenSaveModalWithAction(std::function<void()> onProceed);
};

class TE_API EditorModeSelectorOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
};

class TE_API PlayToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
};

class TE_API PauseToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
};

class TE_API StopToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
};

class TE_API StandaloneToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
};

class TE_API RestartEditorToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
    static void RequestRestart();
    static void RestartEditor();
};

