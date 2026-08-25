#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Texture.hpp"
#include <functional>


enum class ViewportOverlayCorner
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum class ViewportOverlayAlignment
{
    Left,
    Center,
    Right
};

class EditorLayer;

class IViewportOverlay
{
public:
    virtual ~IViewportOverlay() = default;

    virtual void OnClicked(const TEString &itemId) {}
    virtual void OnHovered(const TEString &itemId) {}
    virtual void OnRightClicked(const TEString &itemId) {}
    virtual void OnRenderCustom(const TEString &itemId) {}

    // Gizmo rendering lifecycle hooks
    virtual void OnRenderGizmo(Ref<EditorLayer> editor) {}
    virtual bool OnGizmoDrag(Ref<EditorLayer> editor) { return false; }
    virtual bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) { return false; }
};

class TE_API ViewportOverlayOwnerRegistry
{
public:
    static void RegisterOverlay(TERef<IViewportOverlay> overlay);
    static TEArray<TERef<IViewportOverlay>> GetOverlays();
    static void Clear();
};

template <typename T> struct ViewportOverlayRegisterer
{
    ViewportOverlayRegisterer() { ViewportOverlayOwnerRegistry::RegisterOverlay(CreateRef<T>()); }
};

#define TE_REGISTER_VIEWPORT_OVERLAY(Type) static ViewportOverlayRegisterer<Type> Type##_ViewportOverlayReg;

struct ViewportOverlayItem
{
    TEString id;
    TEString label;
    TEString tooltip;
    ViewportOverlayCorner corner = ViewportOverlayCorner::TopRight;
    ViewportOverlayAlignment alignment = ViewportOverlayAlignment::Right;
    int priority = 0; // Lower numbers render first within group

    TERef<Texture> icon = nullptr;
    IViewportOverlay *ownerOverlay = nullptr;

    std::function<bool()> isVisible = nullptr;
    std::function<bool()> isEnabled = nullptr;
    std::function<bool()> isActive = nullptr; // Highlight active state
    std::function<void()> onClick = nullptr;
    std::function<void()> onRightClick = nullptr;
    std::function<void()> onHover = nullptr;

    // Optional custom delegate for complex overlay widgets
    std::function<void()> onCustomRender = nullptr;
};

class TE_API ViewportOverlayRegistry
{
public:
    static void RegisterItem(const ViewportOverlayItem &item);
    static void UnregisterItem(const TEString &id);
    static TEArray<ViewportOverlayItem> GetItems(ViewportOverlayCorner corner,
                                                  ViewportOverlayAlignment align = ViewportOverlayAlignment::Right);
    static void Clear();
};

