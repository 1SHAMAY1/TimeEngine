#include "Layers/Layer.hpp"

#include "imgui.h"

namespace TE {

    Layer::Layer(const std::string& name)
        : m_DebugName(name)
    {
    }

    Layer::~Layer()
    {
    }

    void Layer::OnAttach()
    {
    }

    void Layer::OnDetach()
    {
    }

    void Layer::OnUpdate()
    {
    }

    void Layer::OnImGuiRender()
    {
    }

    void Layer::OnEvent(class Event& event)
    {
    }

    void Layer::DockTo(bool padding)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        if (!padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

        ImGui::Begin(m_DebugName.c_str(), nullptr, flags); // ⚠️ must have matching ImGui::End

        ImGuiID dockspaceID = ImGui::GetID((m_DebugName + "_Dockspace").c_str());
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End(); // ✅ REQUIRED: matches Begin()

        ImGui::PopStyleVar(padding ? 1 : 2); 
    }

}
