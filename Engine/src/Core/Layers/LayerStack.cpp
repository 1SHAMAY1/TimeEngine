#include "Layers/LayerStack.hpp"

namespace TE {

    LayerStack::LayerStack()
    {
    }

    LayerStack::~LayerStack()
    {
        for (Layer* layer : m_Layers)
            delete layer;

        m_Layers.clear();
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        m_LayerInsertIndex++;
        layer->OnAttach();
    }

    void LayerStack::PushOverlay(Layer* overlay)
    {
        m_Layers.emplace_back(overlay);
        overlay->OnAttach();
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
        if (it != m_Layers.begin() + m_LayerInsertIndex) {
            layer->OnDetach();
            m_Layers.erase(it);
            m_LayerInsertIndex--;
        }
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
        if (it != m_Layers.end()) {
            overlay->OnDetach();
            m_Layers.erase(it);
        }
    }

    void LayerStack::MarkLayerForRemoval(Layer* layer)
    {
        // Check if layer is already marked for removal
        auto it = std::find(m_LayersToRemove.begin(), m_LayersToRemove.end(), layer);
        if (it == m_LayersToRemove.end()) {
            m_LayersToRemove.push_back(layer);
        }
    }

    void LayerStack::MarkOverlayForRemoval(Layer* overlay)
    {
        // Check if overlay is already marked for removal
        auto it = std::find(m_LayersToRemove.begin(), m_LayersToRemove.end(), overlay);
        if (it == m_LayersToRemove.end()) {
            m_LayersToRemove.push_back(overlay);
        }
    }

    void LayerStack::ProcessDeferredRemovals()
    {
        for (Layer* layer : m_LayersToRemove) {
            // Determine if it's a layer or overlay based on position
            auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
            if (it != m_Layers.end()) {
                if (it < m_Layers.begin() + m_LayerInsertIndex) {
                    // It's a layer
                    PopLayer(layer);
                } else {
                    // It's an overlay
                    PopOverlay(layer);
                }
            }
        }
        m_LayersToRemove.clear();
    }
}
