#include "SpriteEditorLayer.hpp"
#include "Core/Log.h"
#include "Input/ShortcutManager.hpp"
#include "SpriteMode.hpp"
#include "Submodes/ISubmode.hpp"
#include "Utils/TimeGUI.hpp"

SpriteEditorLayer::SpriteEditorLayer(const TEString &name) : Layer(name) { m_SpriteMode = CreateScope<SpriteMode>(); }

SpriteEditorLayer::~SpriteEditorLayer() {}

void SpriteEditorLayer::OnAttach()
{
    TE_CORE_INFO("[SpriteEditorLayer] Attached.");
    if (m_SpriteMode)
    {
        m_SpriteMode->OnEnter();
    }

    ShortcutManager::AddListener("SpriteEditorLayer",
                                 [this](const TEString &shortcutId) { return OnShortcut(shortcutId); });
}

void SpriteEditorLayer::OnDetach()
{
    TE_CORE_INFO("[SpriteEditorLayer] Detached.");
    ShortcutManager::RemoveListener("SpriteEditorLayer");

    if (m_SpriteMode)
    {
        m_SpriteMode->OnExit();
    }
}

void SpriteEditorLayer::OnUpdate()
{
    float dt = TimeGUI::GetIO().DeltaTime;
    if (dt > 0.05f)
        dt = 0.05f;

    if (m_SpriteMode)
    {
        m_SpriteMode->OnUpdate(dt);
    }
}

void SpriteEditorLayer::OnTimeGUIRender()
{
    if (!m_SpriteMode)
        return;

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(8, 8));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 6.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(8, 8));

    TimeGUI::Begin("Sprite Studio", nullptr, TimeGUIWindowFlags_NoCollapse | TimeGUIWindowFlags_NoMove);

    // ── Top Submode Navigation Header ──────────────────────────────────────────
    TimeGUI::BeginChild("##SubmodeNavHeader", TEVector2(0, 38), false, TimeGUIWindowFlags_NoScrollbar);

    TEVector4 activeCol(0.20f, 0.45f, 0.85f, 1.0f);
    TEVector4 inactiveCol(0.18f, 0.18f, 0.22f, 1.0f);

    for (int i = 0; i < (int)m_SpriteMode->m_Submodes.size(); i++)
    {
        const auto &submode = m_SpriteMode->m_Submodes[i];
        if (!submode)
            continue;

        bool isActive = (m_SpriteMode->m_ActiveSubmodeIndex == i);
        TimeGUI::PushStyleColor(TimeGUICol_Button, isActive ? activeCol : inactiveCol);

        TEString btnText = TEString(submode->GetIcon()) + " " + submode->GetName();
        if (TimeGUI::Button(btnText.c_str(), TEVector2(140, 30)))
        {
            if (m_SpriteMode->m_ActiveSubmodeIndex != i)
            {
                if (m_SpriteMode->m_ActiveSubmodeIndex >= 0 &&
                    m_SpriteMode->m_ActiveSubmodeIndex < (int)m_SpriteMode->m_Submodes.size())
                {
                    m_SpriteMode->m_Submodes[m_SpriteMode->m_ActiveSubmodeIndex]->OnExit(m_SpriteMode.get());
                }
                m_SpriteMode->m_ActiveSubmodeIndex = i;
                submode->OnEnter(m_SpriteMode.get());
            }
        }
        TimeGUI::PopStyleColor();
        TimeGUI::SameLine();
    }

    // Right-aligned Export Action
    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 130);
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.15f, 0.6f, 0.3f, 1.0f));
    if (TimeGUI::Button("Export PNG", TEVector2(110, 30)))
    {
        m_SpriteMode->m_ShowExportPopup = true;
    }
    TimeGUI::PopStyleColor();

    TimeGUI::EndChild();
    TimeGUI::Separator();

    // ── Active Submode Body Rendering ──────────────────────────────────────────
    if (m_SpriteMode->m_ActiveSubmodeIndex >= 0 &&
        m_SpriteMode->m_ActiveSubmodeIndex < (int)m_SpriteMode->m_Submodes.size())
    {
        m_SpriteMode->m_Submodes[m_SpriteMode->m_ActiveSubmodeIndex]->OnTimeGUIRender(this, m_SpriteMode.get());
    }

    TimeGUI::End();
    TimeGUI::PopStyleVar(3);
}

void SpriteEditorLayer::OnEvent(Event &event) {}

bool SpriteEditorLayer::OnShortcut(const TEString &shortcutId)
{
    if (m_SpriteMode)
    {
        return m_SpriteMode->OnShortcut(shortcutId);
    }
    return false;
}
