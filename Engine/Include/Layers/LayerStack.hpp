#pragma once

#include "Core/PreRequisites.h"
#include "Layer.hpp"


namespace TE {

    class TE_API LayerStack {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);
        
        // Deferred removal methods to prevent crashes during iteration
        void MarkLayerForRemoval(Layer* layer);
        void MarkOverlayForRemoval(Layer* overlay);
        void ProcessDeferredRemovals();

        std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
        std::vector<Layer*>::iterator end()   { return m_Layers.end();   }

    private:
        std::vector<Layer*> m_Layers;
        std::vector<Layer*> m_LayersToRemove;  // Layers marked for deferred removal
        unsigned int m_LayerInsertIndex = 0;
    };

    
} 
