#include "Core/PreRequisites.h"
#include "Core/Scene/CanvasLayerComponent.hpp"

void CanvasLayerComponent::OnAttach() {}

void CanvasLayerComponent::Tick(float deltaTime)
{
    if (!Visible)
        return;

    for (auto &widget : m_Widgets)
    {
        if (widget)
        {
            widget->OnUpdate(deltaTime);
        }
    }
}

void CanvasLayerComponent::Draw()
{
    if (!Visible)
        return;

    for (auto &widget : m_Widgets)
    {
        if (widget && widget->IsVisible())
        {
            widget->Draw();
        }
    }
}

void CanvasLayerComponent::AddWidget(TERef<UIWidget> widget)
{
    if (widget)
    {
        m_Widgets.Add(widget);
    }
}

void CanvasLayerComponent::RemoveWidget(TERef<UIWidget> widget)
{
    if (!widget)
        return;

    for (size_t i = 0; i < m_Widgets.Num(); ++i)
    {
        if (m_Widgets[i] == widget)
        {
            m_Widgets.RemoveAt(i);
            break;
        }
    }
}

void CanvasLayerComponent::ClearWidgets() { m_Widgets.Clear(); }
