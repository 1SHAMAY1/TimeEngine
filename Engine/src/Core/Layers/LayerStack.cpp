#include "Core/PreRequisites.h"
#include "Layers/LayerStack.hpp"
#include "GameFrameWork/GameplayUtils.hpp"


LayerStack::LayerStack() {}

LayerStack::~LayerStack()
{
    for (auto &layer : m_Layers)
    {
        if (layer)
            layer->OnDetach();
    }
    // TERef handles reference-counted destruction automatically - no manual SafeDelete loops
    m_Layers.Clear();
}

void LayerStack::PushLayer(TERef<Layer> layer)
{
    if (!layer)
        return;

    m_Layers.Insert(m_LayerInsertIndex, layer);
    m_LayerInsertIndex++;
    layer->OnAttach();
}

void LayerStack::PushOverlay(TERef<Layer> overlay)
{
    if (!overlay)
        return;

    m_Layers.Add(overlay);
    overlay->OnAttach();
}

void LayerStack::PopLayer(TERef<Layer> layer)
{
    if (!layer)
        return;

    for (size_t i = 0; i < m_LayerInsertIndex; ++i)
    {
        if (m_Layers[i] == layer)
        {
            m_Layers[i]->OnDetach();
            m_Layers.RemoveAt(i);
            m_LayerInsertIndex--;
            break;
        }
    }
}

void LayerStack::PopOverlay(TERef<Layer> overlay)
{
    if (!overlay)
        return;

    for (size_t i = m_LayerInsertIndex; i < m_Layers.Size(); ++i)
    {
        if (m_Layers[i] == overlay)
        {
            m_Layers[i]->OnDetach();
            m_Layers.RemoveAt(i);
            break;
        }
    }
}

void LayerStack::MarkLayerForRemoval(TERef<Layer> layer)
{
    if (!layer)
        return;

    if (!m_LayersToRemove.Contains(layer))
    {
        m_LayersToRemove.Add(layer);
    }
}

void LayerStack::MarkOverlayForRemoval(TERef<Layer> overlay)
{
    if (!overlay)
        return;

    if (!m_LayersToRemove.Contains(overlay))
    {
        m_LayersToRemove.Add(overlay);
    }
}

void LayerStack::ProcessDeferredRemovals()
{
    for (const auto &layer : m_LayersToRemove)
    {
        if (!layer)
            continue;

        for (size_t i = 0; i < m_Layers.Size(); ++i)
        {
            if (m_Layers[i] == layer)
            {
                if (i < m_LayerInsertIndex)
                {
                    PopLayer(layer);
                }
                else
                {
                    PopOverlay(layer);
                }
                break;
            }
        }
    }
    m_LayersToRemove.Clear();
}

void LayerStack::Clear()
{
    for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
    {
        if (*it)
            (*it)->OnDetach();
    }
    m_Layers.Clear();
    m_LayersToRemove.Clear();
    m_LayerInsertIndex = 0;
}

