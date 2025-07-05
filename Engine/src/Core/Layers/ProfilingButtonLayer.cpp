#include "Layers/ProfilingButtonLayer.hpp"
#include "Core/Log.h"
#include "imgui.h"

namespace TE {

    ProfilingButtonLayer::ProfilingButtonLayer()
        : Layer("ProfilingButtonLayer") {
        m_ProfilingLayer = std::make_shared<ProfilingLayer>();
    }

    ProfilingButtonLayer::~ProfilingButtonLayer() {
    }

    void ProfilingButtonLayer::OnAttach() {
        TE_CORE_INFO("ProfilingButtonLayer: OnAttach called.");
        m_ProfilingLayer->OnAttach();
    }

    void ProfilingButtonLayer::OnDetach() {
        TE_CORE_INFO("ProfilingButtonLayer: OnDetach called.");
        m_ProfilingLayer->OnDetach();
    }

    void ProfilingButtonLayer::OnUpdate() {
        m_ProfilingLayer->OnUpdate();
    }

    void ProfilingButtonLayer::OnImGuiRender() {
        RenderProfilingButton();
        m_ProfilingLayer->OnImGuiRender();
    }

    void ProfilingButtonLayer::OnEvent(Event& event) {
        m_ProfilingLayer->OnEvent(event);
    }

    void ProfilingButtonLayer::RenderProfilingButton() {
        if (!m_ShowButton) return;

        // Set button position
        ImGui::SetNextWindowPos(m_ButtonPosition, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(200, 60), ImGuiCond_FirstUseEver);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar;

        if (ImGui::Begin("ProfilingButton", nullptr, windowFlags)) {
            // Button to show/hide profiling window
            if (ImGui::Button("📊 Performance Monitor", ImVec2(180, 40))) {
                if (!m_ProfilingLayer->IsVisible()) {
                    CreateFloatingProfilingWindow();
                } else {
                    m_ProfilingLayer->SetVisible(false);
                }
            }
            
            // Tooltip
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Click to open/close the performance monitoring window\nCan be docked to other windows");
            }
        }
        ImGui::End();
    }

    void ProfilingButtonLayer::CreateFloatingProfilingWindow() {
        // Configure the profiling layer for dockable window
        m_ProfilingLayer->SetFloating(false); // Allow docking
        m_ProfilingLayer->SetVisible(true);
        m_ProfilingLayer->SetWindowTitle("Performance Monitor");
        m_ProfilingLayer->SetWindowPosition(ImVec2(50, 50));
        m_ProfilingLayer->SetWindowSize(ImVec2(350, 250));
        
        // Set some default settings for the profiling window
        m_ProfilingLayer->SetShowDetailedInfo(true);
        m_ProfilingLayer->SetShowGraphs(true);
        m_ProfilingLayer->SetUpdateInterval(0.1f);
        m_ProfilingLayer->SetMaxHistorySize(100);
        
        // Record some initial rendering stats
        m_ProfilingLayer->RecordDrawCall();
        m_ProfilingLayer->RecordTriangle(1);
        m_ProfilingLayer->RecordVertex(3);
        m_ProfilingLayer->RecordShader(1);
    }

} 