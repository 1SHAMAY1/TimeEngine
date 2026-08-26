#include "Core/PreRequisites.h"
#include "UI/UIWidget.hpp"

UIWidget::UIWidget(const TEString &id) : m_ID(id) {}

void UIWidget::AddChild(TERef<UIWidget> child)
{
    if (!child)
        return;

    child->m_Parent = shared_from_this();
    m_Children.Add(child);
}

void UIWidget::RemoveChild(TERef<UIWidget> child)
{
    if (!child)
        return;

    for (int32_t i = 0; i < m_Children.Num(); ++i)
    {
        if (m_Children[i] == child)
        {
            m_Children[i]->m_Parent.reset();
            m_Children.RemoveAt(i);
            break;
        }
    }
}

void UIWidget::ClearChildren()
{
    for (auto &child : m_Children)
    {
        if (child)
            child->m_Parent.reset();
    }
    m_Children.Empty();
}

void UIWidget::Draw()
{
    if (!m_IsVisible)
        return;

    DrawSelf();

    for (auto &child : m_Children)
    {
        if (child && child->IsVisible())
        {
            child->Draw();
        }
    }
}

void UIWidget::OnUpdate(float dt)
{
    for (auto &child : m_Children)
    {
        if (child)
        {
            child->OnUpdate(dt);
        }
    }
}

static TEArray<TERef<UIWidget>> s_ViewportWidgets;

void UIWidget::AddToViewport(int zOrder)
{
    m_ZOrder = zOrder;
    m_IsInViewport = true;
    auto self = shared_from_this();
    for (size_t i = 0; i < s_ViewportWidgets.Num(); ++i)
    {
        if (s_ViewportWidgets[i] == self)
            return;
    }

    s_ViewportWidgets.Add(self);
    // Sort by zOrder ascending
    std::stable_sort(s_ViewportWidgets.begin(), s_ViewportWidgets.end(),
                     [](const TERef<UIWidget> &a, const TERef<UIWidget> &b)
                     { return a->GetZOrder() < b->GetZOrder(); });
}

void UIWidget::RemoveFromViewport()
{
    m_IsInViewport = false;
    auto self = shared_from_this();
    for (size_t i = 0; i < s_ViewportWidgets.Num(); ++i)
    {
        if (s_ViewportWidgets[i] == self)
        {
            s_ViewportWidgets.RemoveAt(i);
            break;
        }
    }
}

void UIWidget::DrawViewportWidgets()
{
    for (auto &widget : s_ViewportWidgets)
    {
        if (widget && widget->IsVisible())
        {
            widget->Draw();
        }
    }
}

void UIWidget::UpdateViewportWidgets(float dt)
{
    for (auto &widget : s_ViewportWidgets)
    {
        if (widget)
        {
            widget->OnUpdate(dt);
        }
    }
}

void UIWidget::ClearViewportWidgets() { s_ViewportWidgets.Clear(); }
