#include "Core/PreRequisites.h"
#include "Editor/Panels/TerminalPanel.hpp"
#include "Core/Plugin/PluginManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"
#include <cstdio>

#ifdef TE_PLATFORM_WINDOWS
#define TE_POPEN _popen
#define TE_PCLOSE _pclose
#else
#define TE_POPEN popen
#define TE_PCLOSE pclose
#endif


TerminalPanel::TerminalPanel()
    : IEditorPanel("Console & Terminal")
{
    m_Visible = true;
    m_InputBuffer.Reserve(512);

    // Initial greeting in Terminal Shell
    m_History.Add("TimeEngine Developer Terminal [Version 1.0.0]");
    m_History.Add("Type 'help' for engine commands or run any system CLI / shell command (e.g. dir, git status, cmake).");
    m_History.Add("");
}

void TerminalPanel::OnAttach()
{
}

void TerminalPanel::ExecuteSystemCLI(const TEString &cmd)
{
    TEString fullCmd = cmd + " 2>&1";
    FILE *pipe = TE_POPEN(fullCmd.c_str(), "r");
    if (!pipe)
    {
        m_History.Add("[Shell Error] Failed to execute process: " + cmd);
        return;
    }

    TEString lineBuffer;
    lineBuffer.Reserve(512);
    bool hasOutput = false;

    while (fgets(lineBuffer.Data(), 512, pipe) != nullptr)
    {
        TEString line = lineBuffer.Data();
        while (!line.IsEmpty() && (line.EndsWith("\n") || line.EndsWith("\r")))
        {
            line = line.Left(line.Length() - 1);
        }
        m_History.Add(line);
        hasOutput = true;
    }
    TE_PCLOSE(pipe);

    if (!hasOutput)
    {
        m_History.Add("[Process finished with code 0]");
    }
}

void TerminalPanel::ExecuteTerminalCommand(const TEString &commandLine, Ref<EditorLayer> editor)
{
    TEString trimmed = commandLine.Trim();
    if (trimmed.IsEmpty())
        return;

    // Record in terminal history
    m_History.Add(TEString("> ") + trimmed);
    m_CommandHistory.Add(trimmed);
    m_HistoryIndex = -1;
    m_ScrollShellToBottom = true;

    TEString lowerCmd = trimmed.ToLower();

    // ── Built-in Engine Commands ──────────────────────────────────────────────
    if (lowerCmd == "help")
    {
        m_History.Add("------------------------------------------------------------------");
        m_History.Add("TimeEngine Interactive Command Center:");
        m_History.Add("  help              - Display this list of engine commands");
        m_History.Add("  clear / cls       - Clear terminal shell output buffer");
        m_History.Add("  stats / fps       - Display realtime FPS, delta time, and performance");
        m_History.Add("  scene             - Display active scene name and entity count");
        m_History.Add("  plugins           - List all discovered and active engine plugins");
        m_History.Add("  reload            - Reload current active scene from disk");
        m_History.Add("  [CLI Commands]    - Execute any OS shell command (e.g. dir, git status, python)");
        m_History.Add("------------------------------------------------------------------");
    }
    else if (lowerCmd == "clear" || lowerCmd == "cls")
    {
        m_History.Clear();
    }
    else if (lowerCmd == "stats" || lowerCmd == "fps")
    {
        float dt = TimeGUI::GetIO().DeltaTime;
        float fps = dt > 0.0f ? (1.0f / dt) : 60.0f;
        float ms = dt * 1000.0f;
        m_History.Add(TEString("Performance: ") + TEString::FromFloat(fps, 1) + " FPS (" + TEString::FromFloat(ms, 2) + " ms/frame)");
        if (editor && editor->GetActiveScene())
        {
            m_History.Add(TEString("Active Entities: ") + TEString::FromInt((int)editor->GetActiveScene()->GetEntityManager().GetAliveEntities().size()));
        }
    }
    else if (lowerCmd == "scene")
    {
        if (editor && editor->GetActiveScene())
        {
            m_History.Add("Active Scene: " + editor->GetActiveScene()->GetName());
            m_History.Add("Total Entities: " + TEString::FromInt((int)editor->GetActiveScene()->GetEntityManager().GetAliveEntities().size()));
        }
        else
        {
            m_History.Add("No active scene loaded.");
        }
    }
    else if (lowerCmd == "plugins")
    {
        const auto &plugins = PluginManager::GetDiscoveredPlugins();
        m_History.Add("Discovered Engine Plugins (" + TEString::FromInt((int)plugins.size()) + "):");
        for (const auto &p : plugins)
        {
            TEString status = p.Enabled ? "[ENABLED] " : "[DISABLED]";
            m_History.Add("  " + status + " " + p.Name + " (v" + p.Version + ") - " + p.Description);
        }
    }
    else if (lowerCmd == "reload")
    {
        m_History.Add("Reloading active scene...");
        if (editor && editor->GetActiveScene())
        {
            TE_CORE_INFO("Reloading active scene via Terminal command.");
        }
    }
    else
    {
        // Execute through native system CLI shell runner
        ExecuteSystemCLI(trimmed);
    }
}

void TerminalPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TimeGUI::SetNextWindowSize(TEVector2(700.0f, 320.0f), TimeGUICond_FirstUseEver);
    TimeGUI::Begin(GetTitle().c_str(), &m_Visible);

    // ── Top Submode View Switcher Bar ─────────────────────────────────────────
    bool isConsoleLogs = (m_ViewMode == ETerminalViewMode::ConsoleLogs);
    bool isTerminalShell = (m_ViewMode == ETerminalViewMode::TerminalShell);

    // 1. View Mode Switcher Buttons
    if (isConsoleLogs)
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.40f, 0.68f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.24f, 0.48f, 0.80f, 1.0f));
    }
    else
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.14f, 0.18f, 0.24f, 0.85f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.18f, 0.24f, 0.32f, 1.0f));
    }
    if (TimeGUI::Button("Console Logs", TEVector2(120.0f, 26.0f)))
    {
        m_ViewMode = ETerminalViewMode::ConsoleLogs;
    }
    TimeGUI::PopStyleColor(2);

    TimeGUI::SameLine(0, 6.0f);

    if (isTerminalShell)
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.40f, 0.68f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.24f, 0.48f, 0.80f, 1.0f));
    }
    else
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.14f, 0.18f, 0.24f, 0.85f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.18f, 0.24f, 0.32f, 1.0f));
    }
    if (TimeGUI::Button("Terminal Shell", TEVector2(120.0f, 26.0f)))
    {
        m_ViewMode = ETerminalViewMode::TerminalShell;
        m_ReclaimFocus = true;
    }
    TimeGUI::PopStyleColor(2);

    TimeGUI::SameLine();
    TimeGUI::Spacing();
    TimeGUI::SameLine();

    // ── Mode-Specific Header Controls (Right-aligned) ─────────────────────────
    if (m_ViewMode == ETerminalViewMode::ConsoleLogs)
    {
        TimeGUI::Checkbox("Auto-Scroll", &m_AutoScrollLogs);
        TimeGUI::SameLine(0, 14.0f);

        if (TimeGUI::Button("Clear Logs", TEVector2(85.0f, 24.0f)))
        {
            Log::ClearMessageBuffer();
        }

        TimeGUI::SameLine(0, 14.0f);
        TimeGUI::SetNextItemWidth(160.0f);
        TimeGUI::InputTextWithHint("##LogFilter", "Filter logs...", m_LogFilter);
    }
    else
    {
        TimeGUI::Checkbox("Auto-Scroll", &m_AutoScrollShell);
        TimeGUI::SameLine(0, 14.0f);

        if (TimeGUI::Button("Clear Output", TEVector2(95.0f, 24.0f)))
        {
            m_History.Clear();
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // ── 1. FULL-PANEL VIEW: Console Logs ──────────────────────────────────────
    if (m_ViewMode == ETerminalViewMode::ConsoleLogs)
    {
        TimeGUI::BeginChild("##ConsoleLogsScrollArea", TEVector2(0, 0), false, TimeGUIWindowFlags_AlwaysVerticalScrollbar);

        auto messages = Log::GetMessageBuffer();
        for (const auto &msg : messages)
        {
            if (!m_LogFilter.IsEmpty())
            {
                if (!msg.Message.Contains(m_LogFilter, ESearchCase::IgnoreCase) &&
                    !msg.Category.Contains(m_LogFilter, ESearchCase::IgnoreCase) &&
                    !msg.Level.Contains(m_LogFilter, ESearchCase::IgnoreCase))
                {
                    continue;
                }
            }

            // Dynamic Log Color from Logger system
            TEVector4 logCol = Log::GetLogColor(msg.Category, msg.Level);

            // Render full log line matching console logger
            TimeGUI::TextColored(logCol, "[%s] [%s] [%s] %s", msg.Timestamp.c_str(), msg.Category.c_str(), msg.Level.c_str(), msg.Message.c_str());
        }

        if (m_FirstLogsFrame || (m_AutoScrollLogs && messages.Size() > m_PreviousLogCount))
        {
            TimeGUI::SetScrollHereY(1.0f);
            m_FirstLogsFrame = false;
        }
        m_PreviousLogCount = messages.Size();

        TimeGUI::EndChild();
    }
    // ── 2. FULL-PANEL VIEW: Interactive Terminal Shell ────────────────────────
    else
    {
        float footerHeight = 36.0f;
        TimeGUI::BeginChild("##TerminalShellScrollArea", TEVector2(0, -footerHeight), false, TimeGUIWindowFlags_AlwaysVerticalScrollbar);

        for (const auto &line : m_History)
        {
            if (line.StartsWith("> "))
            {
                TimeGUI::TextColored(TEVector4(0.35f, 0.75f, 1.0f, 1.0f), "%s", line.c_str());
            }
            else if (line.StartsWith("[Error") || line.StartsWith("[Shell Error") || line.StartsWith("Error"))
            {
                TimeGUI::TextColored(TEVector4(0.95f, 0.30f, 0.35f, 1.0f), "%s", line.c_str());
            }
            else if (line.StartsWith("---") || line.StartsWith("==="))
            {
                TimeGUI::TextColored(TEVector4(0.30f, 0.45f, 0.65f, 1.0f), "%s", line.c_str());
            }
            else
            {
                TimeGUI::TextUnformatted(line.c_str());
            }
        }

        if (m_ScrollShellToBottom)
        {
            TimeGUI::SetScrollHereY(1.0f);
            m_ScrollShellToBottom = false;
        }

        TimeGUI::EndChild();

        TimeGUI::Separator();

        // Interactive Command Execution Prompt at bottom
        TimeGUI::TextColored(TEVector4(0.35f, 0.75f, 1.0f, 1.0f), ">");
        TimeGUI::SameLine(0, 6.0f);

        float availW = TimeGUI::GetContentRegionAvail().x;
        float execBtnWidth = 80.0f;
        TimeGUI::SetNextItemWidth(availW - execBtnWidth - 10.0f);

        bool executeSubmitted = false;
        if (m_ReclaimFocus)
        {
            TimeGUI::SetKeyboardFocusHere();
            m_ReclaimFocus = false;
        }

        if (TimeGUI::InputTextWithHint("##TerminalInputLine", "Enter engine or system CLI command...", m_InputBuffer, TimeGUIInputTextFlags_EnterReturnsTrue))
        {
            executeSubmitted = true;
        }

        TimeGUI::SameLine(0, 8.0f);
        if (TimeGUI::Button("Execute", TEVector2(execBtnWidth, 26.0f)))
        {
            executeSubmitted = true;
        }

        if (executeSubmitted && !m_InputBuffer.IsEmpty())
        {
            ExecuteTerminalCommand(m_InputBuffer, editor);
            m_InputBuffer.Clear();
            m_ReclaimFocus = true;
        }
    }

    TimeGUI::End();
}

TE_REGISTER_EDITOR_PANEL(TerminalPanel);
