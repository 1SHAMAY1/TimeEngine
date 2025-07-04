#include "Layers/EditorLayer.hpp"

namespace TE {
    EditorLayer::EditorLayer(const std::string& name)
        : Layer(name)
    {
    }
    
    EditorLayer::~EditorLayer()
    {
    }

    void EditorLayer::OnImGuiRender()
    {
        DockTo();
    }

}
