#include "Layers/ProfilingButtonLayer.hpp"
#include "Core/Log.h"
#include "Utils/TimeGUI.hpp"

namespace TE
{

ProfilingButtonLayer::ProfilingButtonLayer() : Layer("ProfilingButtonLayer")
{
    m_ProfilingLayer = std::make_shared<ProfilingLayer>();
}

ProfilingButtonLayer::~ProfilingButtonLayer() {}

void ProfilingButtonLayer::OnAttach()
{
    TE_CORE_INFO("ProfilingButtonLayer: OnAttach called.");
    m_ProfilingLayer->OnAttach();
}

void ProfilingButtonLayer::OnDetach()
{
    TE_CORE_INFO("ProfilingButtonLayer: OnDetach called.");
    m_ProfilingLayer->OnDetach();
}

void ProfilingButtonLayer::OnUpdate() { m_ProfilingLayer->OnUpdate(); }

void ProfilingButtonLayer::OnTimeGUIRender()
{
    RenderProfilingButton();
    m_ProfilingLayer->OnTimeGUIRender();
}

void ProfilingButtonLayer::OnEvent(Event &event) { m_ProfilingLayer->OnEvent(event); }

void ProfilingButtonLayer::RenderProfilingButton()
{
    if (!m_ShowButton)
        return;

    // Set button position
    TimeGUI::SetNextWindowPos(m_ButtonPosition, TimeGUICond_FirstUseEver);
    TimeGUI::SetNextWindowSize(TEVector2(200, 60), TimeGUICond_FirstUseEver);

    TimeGUIWindowFlags windowFlags = TimeGUIWindowFlags_NoCollapse | TimeGUIWindowFlags_NoResize |
                                     TimeGUIWindowFlags_NoDocking | TimeGUIWindowFlags_NoTitleBar;

    if (TimeGUI::Begin("ProfilingButton", nullptr, windowFlags))
    {
        // Button to show/hide profiling window
        if (TimeGUI::Button("📊 Performance Monitor", TEVector2(180, 40)))
        {
            if (!m_ProfilingLayer->IsVisible())
            {
                CreateFloatingProfilingWindow();
            }
            else
            {
                m_ProfilingLayer->SetVisible(false);
            }
        }

        // Tooltip
        if (TimeGUI::IsItemHovered())
        {
            TimeGUI::SetTooltip(
                "Click to open/close the performance monitoring window\nCan be docked to other windows");
        }
    }
    TimeGUI::End();
}

void ProfilingButtonLayer::CreateFloatingProfilingWindow()
{
    // Configure the profiling layer for dockable window
    m_ProfilingLayer->SetFloating(false); // Allow docking
    m_ProfilingLayer->SetVisible(true);
    m_ProfilingLayer->SetWindowTitle("Performance Monitor");
    m_ProfilingLayer->SetWindowPosition(TEVector2(50, 50));
    m_ProfilingLayer->SetWindowSize(TEVector2(350, 250));

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

} // namespace TE