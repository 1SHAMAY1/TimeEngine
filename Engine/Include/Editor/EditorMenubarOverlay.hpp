#pragma once
#include "Core/PreRequisites.h"
#include "Editor/EditorMenubarRegistry.hpp"


class EditorLayer;

class TE_API IEditorMenubarOverlay
{
public:
    virtual ~IEditorMenubarOverlay() = default;
    virtual void RegisterMenubarItems(Ref<EditorLayer> editor) = 0;
};

class TE_API EditorMenubarOverlayRegistry
{
public:
    static void RegisterOverlay(TERef<IEditorMenubarOverlay> overlay);
    static TEArray<TERef<IEditorMenubarOverlay>> GetOverlays();
    static void Clear();
};

template <typename T> struct EditorMenubarOverlayRegisterer
{
    EditorMenubarOverlayRegisterer() { EditorMenubarOverlayRegistry::RegisterOverlay(CreateRef<T>()); }
};

#define TE_REGISTER_MENUBAR_OVERLAY(Type) static EditorMenubarOverlayRegisterer<Type> Type##_MenubarOverlayReg;

// Standard Decentralized Menubar Overlays
class TE_API FileMenubarOverlay : public IEditorMenubarOverlay
{
public:
    void RegisterMenubarItems(Ref<EditorLayer> editor) override;
};

class TE_API EditMenubarOverlay : public IEditorMenubarOverlay
{
public:
    void RegisterMenubarItems(Ref<EditorLayer> editor) override;
};

