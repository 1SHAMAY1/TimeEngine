#pragma once
#include "Core/Log.h"
#include "Editor/Panels/IEditorPanel.hpp"

enum class ETerminalViewMode
{
    ConsoleLogs = 0,
    TerminalShell = 1
};

class TE_API TerminalPanel : public IEditorPanel
{
public:
    TerminalPanel();
    TEString GetID() const override { return "Terminal"; }
    TEString GetTitle() const override { return "Console & Terminal"; }
    void OnAttach() override;
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;

private:
    void ExecuteTerminalCommand(const TEString &commandLine, Ref<EditorLayer> editor);
    void ExecuteSystemCLI(const TEString &cmd);

    ETerminalViewMode m_ViewMode = ETerminalViewMode::ConsoleLogs;

    // Console Logs View State
    TEString m_LogFilter;
    int m_SeverityFilter = 0; // 0=All, 1=Info, 2=Warn, 3=Error, 4=Debug
    bool m_AutoScrollLogs = true;
    size_t m_PreviousLogCount = 0;
    bool m_FirstLogsFrame = true;

    // Terminal Shell View State
    TEString m_InputBuffer;
    TEArray<TEString> m_History;
    TEArray<TEString> m_CommandHistory;
    int m_HistoryIndex = -1;
    bool m_AutoScrollShell = true;
    bool m_ScrollShellToBottom = false;
    bool m_ReclaimFocus = false;
};
