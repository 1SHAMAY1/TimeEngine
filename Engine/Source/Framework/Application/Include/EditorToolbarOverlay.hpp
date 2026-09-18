#pragma once

#include "PreRequisites.h"
#include "EditorToolbarRegistry.hpp"

class EditorLayer;

// ── Base Interface ─────────────────────────────────────────────────────────────
class TE_API IEditorToolbarOverlay
{
public:
    virtual ~IEditorToolbarOverlay() = default;
    virtual void RegisterToolbarItems(Ref<EditorLayer> editor) = 0;
    virtual void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) {}
    virtual void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) {}
    virtual bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) { return false; }
};

// ── Decentralized Registry ─────────────────────────────────────────────────────
// Each toolbar button registers itself by placing TE_REGISTER_TOOLBAR_OVERLAY(ClassName)
// in its own .cpp file. No hardcoded list required here.
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

#define TE_REGISTER_TOOLBAR_OVERLAY(Type) inline EditorToolbarOverlayRegisterer<Type> Type##_ToolbarOverlayReg;
