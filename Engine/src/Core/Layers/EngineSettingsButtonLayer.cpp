#include "Layers/EngineSettingsButtonLayer.hpp"
#include "Core/Log.h"
#include "imgui.h"

namespace TE {

    EngineSettingsButtonLayer::EngineSettingsButtonLayer()
        : Layer("EngineSettingsButtonLayer") {
        m_EngineSettingsLayer = std::make_shared<EngineSettingsLayer>();
    }

    EngineSettingsButtonLayer::~EngineSettingsButtonLayer() {
    }

    void EngineSettingsButtonLayer::OnAttach() {
        TE_CORE_INFO("EngineSettingsButtonLayer: OnAttach called.");
        m_EngineSettingsLayer->OnAttach();
    }

    void EngineSettingsButtonLayer::OnDetach() {
        TE_CORE_INFO("EngineSettingsButtonLayer: OnDetach called.");
        m_EngineSettingsLayer->OnDetach();
    }

    void EngineSettingsButtonLayer::OnUpdate() {
        m_EngineSettingsLayer->OnUpdate();
    }

    void EngineSettingsButtonLayer::OnImGuiRender() {
        RenderEngineSettingsButton();
        m_EngineSettingsLayer->OnImGuiRender();
    }

    void EngineSettingsButtonLayer::OnEvent(Event& event) {
        m_EngineSettingsLayer->OnEvent(event);
    }

    void EngineSettingsButtonLayer::RenderEngineSettingsButton() {
        if (!m_ShowButton) return;

        // Set button position
        ImGui::SetNextWindowPos(m_ButtonPosition, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(200, 60), ImGuiCond_FirstUseEver);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar;

        if (ImGui::Begin("EngineSettingsButton", nullptr, windowFlags)) {
            // Button to show/hide engine settings window
            if (ImGui::Button("⚙️ Engine Settings", ImVec2(180, 40))) {
                if (!m_EngineSettingsLayer->IsVisible()) {
                    CreateFloatingEngineSettingsWindow();
                } else {
                    m_EngineSettingsLayer->SetVisible(false);
                }
            }
            
            // Tooltip
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Click to open/close the engine settings window\nConfigure frame rate, logging, and other engine settings");
            }
        }
        ImGui::End();
    }

    void EngineSettingsButtonLayer::CreateFloatingEngineSettingsWindow() {
        // Configure the engine settings layer for dockable window
        m_EngineSettingsLayer->SetVisible(true);
        m_EngineSettingsLayer->SetWindowTitle("Engine Settings");
        m_EngineSettingsLayer->SetWindowPosition(ImVec2(100, 100));
        m_EngineSettingsLayer->SetWindowSize(ImVec2(600, 500));
        
        TE_CORE_INFO("Engine settings window opened");
    }

} // namespace TE 