#include "Layers/Layer.hpp"
#include "Utils/TimeGUI.hpp"

#include "Utils/TimeGUI.hpp"

namespace TE
{

Layer::Layer(const std::string &name) : m_DebugName(name) {}

Layer::~Layer() {}

void Layer::OnAttach() {}

void Layer::OnDetach() {}

void Layer::OnUpdate() {}

void Layer::OnTimeGUIRender() {}

void Layer::OnEvent(class Event &event) {}

void Layer::DockTo(bool padding)
{
    TimeGUIViewport viewport = TimeGUI::GetMainViewport();

    TimeGUI::SetNextWindowPos(viewport.Pos);
    TimeGUI::SetNextWindowSize(viewport.Size);
    TimeGUI::SetNextWindowViewport(viewport.ID);

    TimeGUIWindowFlags flags = TimeGUIWindowFlags_NoTitleBar | TimeGUIWindowFlags_NoCollapse |
                               TimeGUIWindowFlags_NoResize | TimeGUIWindowFlags_NoMove |
                               TimeGUIWindowFlags_NoBringToFrontOnFocus | TimeGUIWindowFlags_NoNavFocus |
                               TimeGUIWindowFlags_MenuBar | TimeGUIWindowFlags_NoDocking;

    if (!padding)
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(0, 0));

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, 0.0f);

    TimeGUI::Begin(m_DebugName.c_str(), nullptr, flags); // ⚠️ must have matching TimeGUI::End

    unsigned int dockspaceID = TimeGUI::GetID((m_DebugName + "_Dockspace").c_str());
    TimeGUI::DockSpace(dockspaceID, TEVector2(0.0f, 0.0f), 0);

    TimeGUI::End(); // ✅ REQUIRED: matches Begin()

    TimeGUI::PopStyleVar(padding ? 1 : 2);
}

} // namespace TE
