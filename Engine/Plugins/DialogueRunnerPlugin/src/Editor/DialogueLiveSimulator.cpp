#pragma once
#include "DialogueLiveSimulator.hpp"
#include "Utils/TimeGUI.hpp"
#include <sstream>


void DialogueLiveSimulator::StartSimulation(const DialogueGraph &graph)
{
    m_Story.LoadFromGraph(graph);
    m_bIsSimulating = true;
    m_LastText.Clear();
    m_LastSpeaker.Clear();
}

void DialogueLiveSimulator::StopSimulation()
{
    m_bIsSimulating = false;
    m_Story.Reset();
    m_LastText.Clear();
    m_LastSpeaker.Clear();
}

void DialogueLiveSimulator::DrawSimulatorPanel(const DialogueGraph &graph)
{
    if (!m_bIsSimulating)
    {
        if (TimeGUI::Button("▶  Start Simulation"))
        {
            StartSimulation(graph);
        }
        return;
    }

    // Header bar
    TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Text, TEColor(0.2f, 0.75f, 0.3f, 1.0f));
    TimeGUI::Text("● LIVE SIMULATION RUNNING");
    TimeGUI::PopStyleColor();
    TimeGUI::SameLine();
    if (TimeGUI::Button("■  Stop"))
    {
        StopSimulation();
        return;
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("↺  Restart"))
    {
        StartSimulation(graph);
    }

    TimeGUI::Separator();

    // Current Dialogue Output
    if (!m_LastSpeaker.empty())
    {
        TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Text, TEColor(0.4f, 0.8f, 1.0f, 1.0f));
        TimeGUI::Text("%s:", m_LastSpeaker.c_str());
        TimeGUI::PopStyleColor();
    }

    if (!m_LastText.empty())
    {
        TimeGUI::TextWrapped("\"%s\"", m_LastText.c_str());
    }
    else if (!m_Story.CanContinue() && m_Story.GetCurrentChoices().empty())
    {
        TimeGUI::TextDisabled("[ End of Conversation ]");
    }

    TimeGUI::Separator();

    // Continue Button
    auto choices = m_Story.GetCurrentChoices();

    if (!choices.empty())
    {
        TimeGUI::Text("Choose a response:");
        TimeGUI::Spacing();

        for (const auto &choice : choices)
        {
            TEString label = "[" + TEString::FromInt(choice.Index + 1) + "] " + choice.Text;
            TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Button, TEColor(0.25f, 0.2f, 0.05f, 1.0f));
            TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ButtonHovered, TEColor(0.45f, 0.35f, 0.05f, 1.0f));
            TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Text, TEColor(0.95f, 0.85f, 0.3f, 1.0f));
            if (TimeGUI::Button(label.c_str(), TEVector2(-1.0f, 0.0f)))
            {
                m_Story.ChooseChoiceIndex(choice.Index);
                m_LastText.Clear();
                m_LastSpeaker.Clear();
                if (m_Story.CanContinue())
                {
                    m_LastText = m_Story.Continue();
                    m_LastSpeaker = m_Story.GetCurrentSpeaker();
                }
            }
            TimeGUI::PopStyleColor(3);
        }
    }
    else if (m_Story.CanContinue())
    {
        if (TimeGUI::Button("► Continue", TEVector2(-1.0f, 0.0f)))
        {
            m_LastText = m_Story.Continue();
            m_LastSpeaker = m_Story.GetCurrentSpeaker();
        }
    }

    TimeGUI::Separator();

    // Live Blackboard Inspector
    if (TimeGUI::CollapsingHeader("Blackboard Variables"))
    {
        const auto &vars = m_Story.GetBlackboard().GetAllVariables();
        if (vars.Size() == 0)
        {
            TimeGUI::TextDisabled("(No variables in blackboard)");
        }
        else
        {
            for (auto it = vars.begin(); it != vars.end(); ++it)
            {
                TimeGUI::Text("%s: %s", it->first.c_str(), it->second.AsString().c_str());
            }
        }
    }

    if (TimeGUI::CollapsingHeader("Active Quests"))
    {
        const auto &quests = m_Story.GetQuestManager().GetAllQuests();
        if (quests.Size() == 0)
        {
            TimeGUI::TextDisabled("(No quests registered)");
        }
        else
        {
            const char *statusStr[] = {"Not Started", "Active", "Completed", "Failed"};
            for (auto it = quests.begin(); it != quests.end(); ++it)
            {
                int s = static_cast<int>(it->second.Status);
                TimeGUI::Text("%s: %s", it->first.c_str(), statusStr[s]);
            }
        }
    }
}

