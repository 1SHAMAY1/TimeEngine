#pragma once

#include "Layer.hpp"

namespace TE {
    class TE_API EditorLayer : public Layer {
    public:
        EditorLayer(const std::string& name = "Editor Layer");
        
        virtual ~EditorLayer();

        virtual void OnImGuiRender() override;
    };
}
