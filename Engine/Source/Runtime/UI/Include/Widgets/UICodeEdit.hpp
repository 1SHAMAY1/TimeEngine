#pragma once

#include "UIWidget.hpp"
#include "Widgets/UISearchBar.hpp"
#include <functional>
#include <map>

enum class ECodeLanguage
{
    TScript,
    GLSL,
    Plain
};

struct TextCursorPos
{
    int Line = 0;
    int Col = 0;

    bool operator==(const TextCursorPos &o) const { return Line == o.Line && Col == o.Col; }
    bool operator!=(const TextCursorPos &o) const { return !(*this == o); }
    bool operator<(const TextCursorPos &o) const
    {
        if (Line != o.Line)
            return Line < o.Line;
        return Col < o.Col;
    }
};

class TE_API UICodeEdit : public UIWidget
{
public:
    UICodeEdit(const TEString &id = "##UICodeEdit");
    virtual ~UICodeEdit();

    void DrawSelf() override;

    // Text & Content Management
    const TEString &GetText() const { return m_FullText; }
    void SetText(const TEString &text);
    void Clear();

    // Language & Theme
    void SetLanguage(ECodeLanguage lang) { m_Language = lang; }
    ECodeLanguage GetLanguage() const { return m_Language; }

    // Error & Diagnostic Markers
    void SetErrorMarker(int line, const TEString &message = "");
    void ClearErrorMarkers();
    int GetErrorLine() const { return m_ErrorLine; }
    const TEString &GetErrorMessage() const { return m_ErrorMessage; }

    // Breakpoint Management (Godot-Style)
    void ToggleBreakpoint(int line);
    void SetBreakpoint(int line, bool enabled);
    bool HasBreakpoint(int line) const;
    void ClearAllBreakpoints();
    const std::map<int, bool> &GetBreakpoints() const { return m_Breakpoints; }

    // Search Bar Controls
    void SetShowSearchBar(bool show) { m_ShowSearchBar = show; }
    bool IsShowSearchBar() const { return m_ShowSearchBar; }
    void ToggleSearchBar() { m_ShowSearchBar = !m_ShowSearchBar; }

    // Text Editing Operations
    void InsertTextAtCursor(const TEString &text);
    void DeleteSelection();
    void SelectAll();
    void Copy();
    void Cut();
    void Paste();

    // Focus & Shortcuts
    bool IsFocused() const { return m_IsFocused; }
    void SetFocused(bool focused) { m_IsFocused = focused; }
    bool OnShortcut(const TEString &shortcutId);

    // Cursor Navigation
    void SetCursor(int line, int col);
    TextCursorPos GetCursor() const { return m_Cursor; }

    // Event Delegates
    std::function<void(const TEString &)> OnTextChanged;
    std::function<void(int line)> OnLineClicked;
    std::function<void(int line, bool enabled)> OnBreakpointToggled;

private:
    void SyncFullTextFromLines();
    void HandleSearchQuery(const TEString &query);
    void HandleKeyboardInput();
    void HandleMouseInput(const TEVector2 &canvasMin, const TEVector2 &textMin, float charWidth, float lineHeight,
                          float scrollY);
    void ClampCursor();
    bool HasSelection() const;
    void ClearSelection();
    void GetNormalizedSelection(TextCursorPos &outStart, TextCursorPos &outEnd) const;

    TEString m_FullText;
    TEArray<TEString> m_Lines;
    TextCursorPos m_Cursor;
    TextCursorPos m_SelectionStart;
    TextCursorPos m_SelectionEnd;
    bool m_IsSelecting = false;

    // Breakpoints
    std::map<int, bool> m_Breakpoints;
    int m_HoveredGutterLine = 0;

    // Error diagnostic
    int m_ErrorLine = 0;
    TEString m_ErrorMessage;
    ECodeLanguage m_Language = ECodeLanguage::TScript;

    UISearchBar m_SearchBar;
    bool m_ShowSearchBar = false;
    int m_SearchMatchLine = -1;

    float m_BlinkTimer = 0.0f;
    bool m_IsFocused = false;
    TEString m_ListenerName;
};
