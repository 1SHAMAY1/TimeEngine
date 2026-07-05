#include "Utils/TimeGUI.hpp"
#include "Layers/EngineSettingsButtonLayer.hpp"
#include "Core/Log.h"
#include "Utils/TimeGUI.hpp"

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

    void EngineSettingsButtonLayer::OnTimeGUIRender() {
        RenderEngineSettingsButton();
        m_EngineSettingsLayer->OnTimeGUIRender();
    }

    void EngineSettingsButtonLayer::OnEvent(Event& event) {
        m_EngineSettingsLayer->OnEvent(event);
    }

    void EngineSettingsButtonLayer::RenderEngineSettingsButton() {
        if (!m_ShowButton) return;

        // Set button position
        TimeGUI::SetNextWindowPos(m_ButtonPosition, TimeGUICond_FirstUseEver);
        TimeGUI::SetNextWindowSize(TEVector2(200, 60), TimeGUICond_FirstUseEver);
        
        TimeGUIWindowFlags windowFlags = TimeGUIWindowFlags_NoCollapse | TimeGUIWindowFlags_NoResize |
                                        TimeGUIWindowFlags_NoDocking | TimeGUIWindowFlags_NoTitleBar;

        if (TimeGUI::Begin("EngineSettingsButton", nullptr, windowFlags)) {
            // Button to show/hide engine settings window
            if (TimeGUI::Button("⚙️ Engine Settings", TEVector2(180, 40))) {
                if (!m_EngineSettingsLayer->IsVisible()) {
                    CreateFloatingEngineSettingsWindow();
                } else {
                    m_EngineSettingsLayer->SetVisible(false);
                }
            }
            
            // Tooltip
            if (TimeGUI::IsItemHovered()) {
                TimeGUI::SetTooltip("Click to open/close the engine settings window\nConfigure frame rate, logging, and other engine settings");
            }
        }
        TimeGUI::End();
    }

    void EngineSettingsButtonLayer::CreateFloatingEngineSettingsWindow() {
        // Configure the engine settings layer for dockable window
        m_EngineSettingsLayer->SetVisible(true);
        m_EngineSettingsLayer->SetWindowTitle("Engine Settings");
        m_EngineSettingsLayer->SetWindowPosition(TEVector2(100, 100));
        m_EngineSettingsLayer->SetWindowSize(TEVector2(600, 500));
        
        TE_CORE_INFO("Engine settings window opened");
    }

} // namespace TE 