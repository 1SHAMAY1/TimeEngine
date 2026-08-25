#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <functional>


class TE_API UIWidget : public std::enable_shared_from_this<UIWidget>
{
public:
    UIWidget(const TEString &id = "");
    virtual ~UIWidget() = default;

    // Hierarchy Management
    void AddChild(TERef<UIWidget> child);
    void RemoveChild(TERef<UIWidget> child);
    void ClearChildren();
    const TEArray<TERef<UIWidget>>& GetChildren() const { return m_Children; }
    TERef<UIWidget> GetParent() const { return m_Parent.lock(); }

    // Transform & Layout
    void SetPosition(const TEVector2 &pos) { m_Position = pos; }
    const TEVector2& GetPosition() const { return m_Position; }

    void SetSize(const TEVector2 &size) { m_Size = size; }
    const TEVector2& GetSize() const { return m_Size; }

    void SetVisible(bool visible) { m_IsVisible = visible; }
    bool IsVisible() const { return m_IsVisible; }

    void SetEnabled(bool enabled) { m_IsEnabled = enabled; }
    bool IsEnabled() const { return m_IsEnabled; }

    const TEString& GetID() const { return m_ID; }
    void SetID(const TEString &id) { m_ID = id; }

    // Core Drawing & Updates
    virtual void Draw();
    virtual void OnUpdate(float dt);

    // Viewport Management
    void AddToViewport(int zOrder = 0);
    void RemoveFromViewport();
    bool IsInViewport() const { return m_IsInViewport; }
    int GetZOrder() const { return m_ZOrder; }

    static void DrawViewportWidgets();
    static void UpdateViewportWidgets(float dt);
    static void ClearViewportWidgets();

    // Event Callbacks
    std::function<void(UIWidget*)> OnHovered;
    std::function<void(UIWidget*)> OnUnhovered;
    std::function<void(UIWidget*)> OnClicked;

protected:
    virtual void DrawSelf() = 0;

    TEString m_ID;
    TEVector2 m_Position = {0.0f, 0.0f};
    TEVector2 m_Size = {0.0f, 0.0f};
    bool m_IsVisible = true;
    bool m_IsEnabled = true;
    bool m_IsInViewport = false;
    int m_ZOrder = 0;

    TEWeakRef<UIWidget> m_Parent;
    TEArray<TERef<UIWidget>> m_Children;
};

