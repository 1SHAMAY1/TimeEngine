#include "Core/PreRequisites.h"
#include "UI/Widgets/UICodeEdit.hpp"
#include "Editor/TScriptSyntaxHighlighter.hpp"
#include "Input/ShortcutManager.hpp"
#include "Utils/TimeGUI.hpp"

UICodeEdit::UICodeEdit(const TEString &id)
    : UIWidget(id), m_SearchBar("Find in script (fuzzy matching)...", "##CodeEditSearch")
{
    m_SearchBar.OnQueryChanged = [this](const TEString &query) { HandleSearchQuery(query); };
    m_SearchBar.OnSearchSubmitted = [this](const TEString &query) { HandleSearchQuery(query); };

    m_Lines.push_back("");
    m_FullText = "";

    m_ListenerName = "UICodeEdit_" + m_ID;
    ShortcutManager::AddListener(m_ListenerName,
                                 [this](const TEString &shortcutId) -> bool { return OnShortcut(shortcutId); });
}

UICodeEdit::~UICodeEdit()
{
    if (!m_ListenerName.empty())
    {
        ShortcutManager::RemoveListener(m_ListenerName);
    }
}

bool UICodeEdit::OnShortcut(const TEString &shortcutId)
{
    if (!m_IsFocused)
        return false;

    if (shortcutId == "Editor_SelectAll")
    {
        SelectAll();
        return true;
    }
    if (shortcutId == "Editor_Copy")
    {
        Copy();
        return true;
    }
    if (shortcutId == "Editor_Paste")
    {
        Paste();
        return true;
    }
    if (shortcutId == "Editor_Cut")
    {
        Cut();
        return true;
    }

    return false;
}

void UICodeEdit::SetText(const TEString &text)
{
    m_FullText = text;
    m_Lines = text.Split('\n');
    if (m_Lines.empty())
    {
        m_Lines.push_back("");
    }

    for (auto &line : m_Lines)
    {
        if (!line.empty() && line[line.length() - 1] == '\r')
        {
            line = line.substr(0, line.length() - 1);
        }
    }

    ClampCursor();
    ClearSelection();
}

void UICodeEdit::Clear()
{
    m_Lines.clear();
    m_Lines.push_back("");
    m_FullText = "";
    m_Cursor = {0, 0};
    ClearSelection();
    ClearErrorMarkers();
    ClearAllBreakpoints();
}

void UICodeEdit::SetErrorMarker(int line, const TEString &message)
{
    m_ErrorLine = line;
    m_ErrorMessage = message;
}

void UICodeEdit::ClearErrorMarkers()
{
    m_ErrorLine = 0;
    m_ErrorMessage = "";
}

void UICodeEdit::ToggleBreakpoint(int line)
{
    if (line <= 0)
        return;

    bool newState = !m_Breakpoints[line];
    m_Breakpoints[line] = newState;

    if (OnBreakpointToggled)
    {
        OnBreakpointToggled(line, newState);
    }
}

void UICodeEdit::SetBreakpoint(int line, bool enabled)
{
    if (line <= 0)
        return;

    m_Breakpoints[line] = enabled;
    if (OnBreakpointToggled)
    {
        OnBreakpointToggled(line, enabled);
    }
}

bool UICodeEdit::HasBreakpoint(int line) const
{
    auto it = m_Breakpoints.find(line);
    if (it != m_Breakpoints.end())
    {
        return it->second;
    }
    return false;
}

void UICodeEdit::ClearAllBreakpoints() { m_Breakpoints.clear(); }

void UICodeEdit::SetCursor(int line, int col)
{
    m_Cursor.Line = line;
    m_Cursor.Col = col;
    ClampCursor();
}

void UICodeEdit::ClampCursor()
{
    if (m_Lines.empty())
    {
        m_Lines.push_back("");
    }

    if (m_Cursor.Line < 0)
        m_Cursor.Line = 0;
    if (m_Cursor.Line >= (int)m_Lines.size())
        m_Cursor.Line = (int)m_Lines.size() - 1;

    int lineLen = (int)m_Lines[m_Cursor.Line].length();
    if (m_Cursor.Col < 0)
        m_Cursor.Col = 0;
    if (m_Cursor.Col > lineLen)
        m_Cursor.Col = lineLen;
}

bool UICodeEdit::HasSelection() const
{
    return (m_SelectionStart.Line >= 0 && m_SelectionEnd.Line >= 0 && m_SelectionStart != m_SelectionEnd);
}

void UICodeEdit::ClearSelection()
{
    m_SelectionStart = {-1, -1};
    m_SelectionEnd = {-1, -1};
    m_IsSelecting = false;
}

void UICodeEdit::GetNormalizedSelection(TextCursorPos &outStart, TextCursorPos &outEnd) const
{
    if (!HasSelection())
    {
        outStart = m_Cursor;
        outEnd = m_Cursor;
        return;
    }

    if (m_SelectionStart < m_SelectionEnd)
    {
        outStart = m_SelectionStart;
        outEnd = m_SelectionEnd;
    }
    else
    {
        outStart = m_SelectionEnd;
        outEnd = m_SelectionStart;
    }
}

void UICodeEdit::SyncFullTextFromLines()
{
    m_FullText = "";
    for (size_t i = 0; i < m_Lines.size(); i++)
    {
        if (i > 0)
            m_FullText += "\n";
        m_FullText += m_Lines[i];
    }

    if (OnTextChanged)
    {
        OnTextChanged(m_FullText);
    }
}

void UICodeEdit::HandleSearchQuery(const TEString &query)
{
    m_SearchMatchLine = -1;
    if (query.empty())
        return;

    for (size_t i = 0; i < m_Lines.size(); i++)
    {
        if (UISearchBar::FuzzyMatch(query, m_Lines[i]))
        {
            m_SearchMatchLine = (int)i;
            m_Cursor.Line = (int)i;
            m_Cursor.Col = 0;
            break;
        }
    }
}

void UICodeEdit::InsertTextAtCursor(const TEString &text)
{
    if (HasSelection())
    {
        DeleteSelection();
    }

    TEArray<TEString> insertLines = text.Split('\n');
    for (auto &l : insertLines)
    {
        if (!l.empty() && l[l.length() - 1] == '\r')
            l = l.substr(0, l.length() - 1);
    }

    if (insertLines.empty())
        return;

    TEString &currLine = m_Lines[m_Cursor.Line];
    TEString prefix = currLine.substr(0, m_Cursor.Col);
    TEString suffix = currLine.substr(m_Cursor.Col);

    if (insertLines.size() == 1)
    {
        currLine = prefix + insertLines[0] + suffix;
        m_Cursor.Col += (int)insertLines[0].length();
    }
    else
    {
        currLine = prefix + insertLines[0];
        for (size_t i = 1; i < insertLines.size() - 1; i++)
        {
            m_Lines.insert(m_Lines.begin() + m_Cursor.Line + i, insertLines[i]);
        }
        size_t lastIdx = insertLines.size() - 1;
        m_Lines.insert(m_Lines.begin() + m_Cursor.Line + lastIdx, insertLines[lastIdx] + suffix);

        m_Cursor.Line += (int)lastIdx;
        m_Cursor.Col = (int)insertLines[lastIdx].length();
    }

    ClampCursor();
    SyncFullTextFromLines();
}

void UICodeEdit::DeleteSelection()
{
    if (!HasSelection())
        return;

    TextCursorPos start, end;
    GetNormalizedSelection(start, end);

    if (start.Line == end.Line)
    {
        TEString &line = m_Lines[start.Line];
        TEString prefix = line.substr(0, start.Col);
        TEString suffix = line.substr(end.Col);
        line = prefix + suffix;
    }
    else
    {
        TEString firstPrefix = m_Lines[start.Line].substr(0, start.Col);
        TEString lastSuffix = m_Lines[end.Line].substr(end.Col);

        m_Lines[start.Line] = firstPrefix + lastSuffix;
        for (int l = end.Line; l > start.Line; l--)
        {
            m_Lines.erase(m_Lines.begin() + l);
        }
    }

    m_Cursor = start;
    ClearSelection();
    ClampCursor();
    SyncFullTextFromLines();
}

void UICodeEdit::SelectAll()
{
    m_SelectionStart = {0, 0};
    int lastLine = (int)m_Lines.size() - 1;
    m_SelectionEnd = {lastLine, (int)m_Lines[lastLine].length()};
    m_Cursor = m_SelectionEnd;
}

void UICodeEdit::Copy()
{
    if (!HasSelection())
        return;

    TextCursorPos start, end;
    GetNormalizedSelection(start, end);

    TEString selectedText;
    if (start.Line == end.Line)
    {
        selectedText = m_Lines[start.Line].substr(start.Col, end.Col - start.Col);
    }
    else
    {
        selectedText = m_Lines[start.Line].substr(start.Col);
        for (int l = start.Line + 1; l < end.Line; l++)
        {
            selectedText += "\n" + m_Lines[l];
        }
        selectedText += "\n" + m_Lines[end.Line].substr(0, end.Col);
    }

    TimeGUI::SetClipboardText(selectedText);
}

void UICodeEdit::Cut()
{
    if (HasSelection())
    {
        Copy();
        DeleteSelection();
    }
}

void UICodeEdit::Paste()
{
    TEString clip = TimeGUI::GetClipboardText();
    if (!clip.empty())
    {
        InsertTextAtCursor(clip);
    }
}

void UICodeEdit::HandleKeyboardInput()
{
    auto &io = TimeGUI::GetIO();
    bool ctrl = io.KeyCtrl;
    bool shift = io.KeyShift;

    // Shortcuts
    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_A))
    {
        SelectAll();
        return;
    }
    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_C))
    {
        Copy();
        return;
    }
    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_V))
    {
        Paste();
        return;
    }
    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_X))
    {
        Cut();
        return;
    }
    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_F))
    {
        ToggleSearchBar();
        return;
    }

    // Navigation Keys
    auto updateSelection = [this, shift](TextCursorPos oldPos)
    {
        if (shift)
        {
            if (!HasSelection())
            {
                m_SelectionStart = oldPos;
            }
            m_SelectionEnd = m_Cursor;
        }
        else
        {
            ClearSelection();
        }
    };

    TextCursorPos prevPos = m_Cursor;

    if (TimeGUI::IsKeyPressed(TimeGUIKey_LeftArrow))
    {
        if (m_Cursor.Col > 0)
        {
            m_Cursor.Col--;
        }
        else if (m_Cursor.Line > 0)
        {
            m_Cursor.Line--;
            m_Cursor.Col = (int)m_Lines[m_Cursor.Line].length();
        }
        updateSelection(prevPos);
        return;
    }
    if (TimeGUI::IsKeyPressed(TimeGUIKey_RightArrow))
    {
        int lineLen = (int)m_Lines[m_Cursor.Line].length();
        if (m_Cursor.Col < lineLen)
        {
            m_Cursor.Col++;
        }
        else if (m_Cursor.Line + 1 < (int)m_Lines.size())
        {
            m_Cursor.Line++;
            m_Cursor.Col = 0;
        }
        updateSelection(prevPos);
        return;
    }
    if (TimeGUI::IsKeyPressed(TimeGUIKey_UpArrow))
    {
        if (m_Cursor.Line > 0)
        {
            m_Cursor.Line--;
            ClampCursor();
        }
        updateSelection(prevPos);
        return;
    }
    if (TimeGUI::IsKeyPressed(TimeGUIKey_DownArrow))
    {
        if (m_Cursor.Line + 1 < (int)m_Lines.size())
        {
            m_Cursor.Line++;
            ClampCursor();
        }
        updateSelection(prevPos);
        return;
    }
    if (TimeGUI::IsKeyPressed(TimeGUIKey_Home))
    {
        m_Cursor.Col = 0;
        updateSelection(prevPos);
        return;
    }
    if (TimeGUI::IsKeyPressed(TimeGUIKey_End))
    {
        m_Cursor.Col = (int)m_Lines[m_Cursor.Line].length();
        updateSelection(prevPos);
        return;
    }

    // Editing Keys
    if (TimeGUI::IsKeyPressed(TimeGUIKey_Enter))
    {
        if (HasSelection())
        {
            DeleteSelection();
        }

        TEString &currLine = m_Lines[m_Cursor.Line];
        TEString prefix = currLine.substr(0, m_Cursor.Col);
        TEString suffix = currLine.substr(m_Cursor.Col);

        // Auto-indent: inherit whitespace
        TEString indent;
        for (size_t c = 0; c < prefix.length(); c++)
        {
            if (prefix[c] == ' ' || prefix[c] == '\t')
                indent += prefix[c];
            else
                break;
        }

        currLine = prefix;
        m_Lines.insert(m_Lines.begin() + m_Cursor.Line + 1, indent + suffix);
        m_Cursor.Line++;
        m_Cursor.Col = (int)indent.length();
        ClampCursor();
        SyncFullTextFromLines();
        return;
    }

    if (TimeGUI::IsKeyPressed(TimeGUIKey_Tab))
    {
        InsertTextAtCursor("    ");
        return;
    }

    if (TimeGUI::IsKeyPressed(TimeGUIKey_Backspace))
    {
        if (HasSelection())
        {
            DeleteSelection();
        }
        else if (m_Cursor.Col > 0)
        {
            TEString &line = m_Lines[m_Cursor.Line];
            line = line.substr(0, m_Cursor.Col - 1) + line.substr(m_Cursor.Col);
            m_Cursor.Col--;
            SyncFullTextFromLines();
        }
        else if (m_Cursor.Line > 0)
        {
            int prevLen = (int)m_Lines[m_Cursor.Line - 1].length();
            m_Lines[m_Cursor.Line - 1] += m_Lines[m_Cursor.Line];
            m_Lines.erase(m_Lines.begin() + m_Cursor.Line);
            m_Cursor.Line--;
            m_Cursor.Col = prevLen;
            SyncFullTextFromLines();
        }
        return;
    }

    if (TimeGUI::IsKeyPressed(TimeGUIKey_Delete))
    {
        if (HasSelection())
        {
            DeleteSelection();
        }
        else
        {
            TEString &line = m_Lines[m_Cursor.Line];
            int lineLen = (int)line.length();
            if (m_Cursor.Col < lineLen)
            {
                line = line.substr(0, m_Cursor.Col) + line.substr(m_Cursor.Col + 1);
                SyncFullTextFromLines();
            }
            else if (m_Cursor.Line + 1 < (int)m_Lines.size())
            {
                line += m_Lines[m_Cursor.Line + 1];
                m_Lines.erase(m_Lines.begin() + m_Cursor.Line + 1);
                SyncFullTextFromLines();
            }
        }
        return;
    }

    // Direct Character Typing
    auto queue = TimeGUI::GetInputQueueCharacters();
    if (!queue.empty())
    {
        for (unsigned int c : queue)
        {
            if (c >= 32 && c != 127)
            {
                char ch = (char)c;
                InsertTextAtCursor(TEString(&ch, 1));
            }
        }
        TimeGUI::ClearInputQueueCharacters();
    }
}

void UICodeEdit::HandleMouseInput(const TEVector2 &canvasMin, const TEVector2 &textMin, float charWidth,
                                  float lineHeight, float scrollY)
{
    TEVector2 mousePos = TimeGUI::GetMousePos();
    float gutterWidth = 58.0f;
    float breakpointMargin = 18.0f;

    // Detect hovered gutter line for ghost breakpoint circle
    if (mousePos.x >= canvasMin.x && mousePos.x <= canvasMin.x + gutterWidth)
    {
        float relY = mousePos.y - textMin.y;
        int hoverLine = (int)(relY / lineHeight) + 1;
        if (hoverLine >= 1 && hoverLine <= (int)m_Lines.size())
        {
            m_HoveredGutterLine = hoverLine;
        }
        else
        {
            m_HoveredGutterLine = 0;
        }
    }
    else
    {
        m_HoveredGutterLine = 0;
    }

    if (TimeGUI::IsMouseClicked(0) && TimeGUI::IsWindowHovered())
    {
        m_IsFocused = true;

        // Check if clicked inside Breakpoint Gutter Margin (0 to 18px)
        if (mousePos.x >= canvasMin.x && mousePos.x <= canvasMin.x + breakpointMargin)
        {
            float relY = mousePos.y - textMin.y;
            int clickedLine = (int)(relY / lineHeight) + 1;
            if (clickedLine >= 1 && clickedLine <= (int)m_Lines.size())
            {
                ToggleBreakpoint(clickedLine);
                return;
            }
        }

        // Clicked inside code editor text area
        float relY = mousePos.y - textMin.y;
        float relX = mousePos.x - textMin.x;

        int clickLine = (int)(relY / lineHeight);
        if (clickLine < 0)
            clickLine = 0;
        if (clickLine >= (int)m_Lines.size())
            clickLine = (int)m_Lines.size() - 1;

        int clickCol = (int)((relX + charWidth * 0.5f) / charWidth);
        if (clickCol < 0)
            clickCol = 0;
        int maxCol = (int)m_Lines[clickLine].length();
        if (clickCol > maxCol)
            clickCol = maxCol;

        m_Cursor = {clickLine, clickCol};
        m_SelectionStart = m_Cursor;
        m_SelectionEnd = m_Cursor;
        m_IsSelecting = true;

        if (OnLineClicked)
        {
            OnLineClicked(clickLine + 1);
        }
    }

    if (m_IsSelecting && TimeGUI::IsMouseDragging(0))
    {
        float relY = mousePos.y - textMin.y;
        float relX = mousePos.x - textMin.x;

        int dragLine = (int)(relY / lineHeight);
        if (dragLine < 0)
            dragLine = 0;
        if (dragLine >= (int)m_Lines.size())
            dragLine = (int)m_Lines.size() - 1;

        int dragCol = (int)((relX + charWidth * 0.5f) / charWidth);
        if (dragCol < 0)
            dragCol = 0;
        int maxCol = (int)m_Lines[dragLine].length();
        if (dragCol > maxCol)
            dragCol = maxCol;

        m_Cursor = {dragLine, dragCol};
        m_SelectionEnd = m_Cursor;
    }

    if (TimeGUI::IsMouseReleased(0))
    {
        m_IsSelecting = false;
        if (m_SelectionStart == m_SelectionEnd)
        {
            ClearSelection();
        }
    }
}

void UICodeEdit::DrawSelf()
{
    // Search Bar (Ctrl+F)
    if (m_ShowSearchBar)
    {
        TimeGUI::BeginChild("##CodeSearchHeader", TEVector2(0, 36.0f), false);
        {
            float availW = TimeGUI::GetContentRegionAvail().x;
            m_SearchBar.SetSize(TEVector2(availW - 80.0f, 26.0f));
            m_SearchBar.Draw();

            TimeGUI::SameLine();
            if (TimeGUI::Button("Close [X]", TEVector2(70.0f, 24.0f)))
            {
                m_ShowSearchBar = false;
            }
        }
        TimeGUI::EndChild();
        TimeGUI::Separator();
    }

    TEVector2 canvasSize = TEVector2(m_Size.x > 0 ? m_Size.x : 0, m_Size.y > 0 ? m_Size.y : 0);

    TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEColor(0.08f, 0.09f, 0.12f, 1.0f));
    TimeGUI::BeginChild(m_ID.c_str(), canvasSize, true, TimeGUIWindowFlags_HorizontalScrollbar);
    {
        TEVector2 canvasMin = TimeGUI::GetCursorScreenPos();
        TEVector2 avail = TimeGUI::GetContentRegionAvail();

        float lineHeight = TimeGUI::CalcTextSize("A").y + 4.0f;
        float charWidth = TimeGUI::CalcTextSize("M").x;
        float gutterWidth = 58.0f;
        TEVector2 textMin(canvasMin.x + gutterWidth + 8.0f, canvasMin.y + 4.0f);

        // Input Handling
        if (m_IsFocused && TimeGUI::IsWindowFocused())
        {
            TimeGUI::GetIO().WantTextInput = true;
            HandleKeyboardInput();
        }
        else if (TimeGUI::IsMouseClicked(0) && !TimeGUI::IsWindowHovered())
        {
            m_IsFocused = false;
        }

        HandleMouseInput(canvasMin, textMin, charWidth, lineHeight, TimeGUI::GetScrollY());

        // Blink Timer
        m_BlinkTimer += TimeGUI::GetIO().DeltaTime;
        bool showCaret = ((int)(m_BlinkTimer * 2.0f) % 2) == 0;

        TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

        // Background Dimensions
        float totalHeight = (float)m_Lines.size() * lineHeight + 20.0f;
        float totalWidth = avail.x > 800.0f ? avail.x : 800.0f;

        // Draw Gutter Background
        dl.AddRectFilled(canvasMin, TEVector2(canvasMin.x + gutterWidth, canvasMin.y + totalHeight),
                         TIMEGUI_COL32(20, 24, 33, 255));
        dl.AddLine(TEVector2(canvasMin.x + gutterWidth, canvasMin.y),
                   TEVector2(canvasMin.x + gutterWidth, canvasMin.y + totalHeight), TIMEGUI_COL32(40, 46, 60, 255),
                   1.0f);

        // Normalize Selection
        TextCursorPos selStart, selEnd;
        bool hasSel = HasSelection();
        if (hasSel)
        {
            GetNormalizedSelection(selStart, selEnd);
        }

        // Render Lines
        for (size_t i = 0; i < m_Lines.size(); i++)
        {
            int lineNum = (int)(i + 1);
            float lineY = textMin.y + (float)i * lineHeight;
            bool isError = (m_ErrorLine == lineNum);
            bool isCursorLine = (m_Cursor.Line == (int)i);
            bool hasBreakpoint = HasBreakpoint(lineNum);
            bool isHoveredGutter = (m_HoveredGutterLine == lineNum);

            // Active Line Highlight
            if (isCursorLine)
            {
                dl.AddRectFilled(TEVector2(canvasMin.x + gutterWidth, lineY - 2.0f),
                                 TEVector2(canvasMin.x + totalWidth, lineY + lineHeight - 2.0f),
                                 TIMEGUI_COL32(36, 42, 56, 160));
            }

            // Error Line Highlight
            if (isError)
            {
                dl.AddRectFilled(TEVector2(canvasMin.x + gutterWidth, lineY - 2.0f),
                                 TEVector2(canvasMin.x + totalWidth, lineY + lineHeight - 2.0f),
                                 TIMEGUI_COL32(110, 30, 40, 140));
            }

            // Breakpoint Dot (🔴 Solid Red Circle)
            float circleCenterY = lineY + lineHeight * 0.4f;
            if (hasBreakpoint)
            {
                dl.AddCircleFilled(TEVector2(canvasMin.x + 9.0f, circleCenterY), 5.0f, TIMEGUI_COL32(235, 60, 60, 255));
            }
            else if (isHoveredGutter)
            {
                // Ghost Hover Circle
                dl.AddCircle(TEVector2(canvasMin.x + 9.0f, circleCenterY), 5.0f, TIMEGUI_COL32(235, 60, 60, 100), 12,
                             1.5f);
            }

            // Gutter Line Numbers / Error Marker
            if (isError)
            {
                dl.AddText(TEVector2(canvasMin.x + 18.0f, lineY), TIMEGUI_COL32(255, 75, 75, 255), ">>");
                dl.AddText(TEVector2(canvasMin.x + 32.0f, lineY), TIMEGUI_COL32(255, 75, 75, 255),
                           TEString::FromInt(lineNum).c_str());
            }
            else
            {
                dl.AddText(TEVector2(canvasMin.x + 20.0f, lineY), TIMEGUI_COL32(110, 120, 135, 255),
                           TEString::FromInt(lineNum).c_str());
            }

            // Selection Background on this line
            if (hasSel && (int)i >= selStart.Line && (int)i <= selEnd.Line)
            {
                int startCol = ((int)i == selStart.Line) ? selStart.Col : 0;
                int endCol = ((int)i == selEnd.Line) ? selEnd.Col : (int)m_Lines[i].length() + 1;

                float selX1 = textMin.x + (float)startCol * charWidth;
                float selX2 = textMin.x + (float)endCol * charWidth;
                dl.AddRectFilled(TEVector2(selX1, lineY - 1.0f), TEVector2(selX2, lineY + lineHeight - 1.0f),
                                 TIMEGUI_COL32(45, 85, 145, 180));
            }

            // Syntax Highlighted Text
            TEString lineText = m_Lines[i];
            auto tokens = TScriptSyntaxHighlighter::Highlight(lineText);
            float currX = textMin.x;

            for (const auto &tok : tokens)
            {
                if (tok.text == "\n")
                    continue;

                dl.AddText(TEVector2(currX, lineY), tok.color, tok.text);
                currX += (float)tok.text.length() * charWidth;
            }

            // Caret on Cursor Line
            if (isCursorLine && showCaret && m_IsFocused)
            {
                float caretX = textMin.x + (float)m_Cursor.Col * charWidth;
                dl.AddLine(TEVector2(caretX, lineY - 1.0f), TEVector2(caretX, lineY + lineHeight - 3.0f),
                           TIMEGUI_COL32(255, 255, 255, 255), 1.8f);
            }
        }

        // Invisible Item to reserve scroll area
        TimeGUI::Dummy(TEVector2(totalWidth, totalHeight));
    }
    TimeGUI::EndChild();
    TimeGUI::PopStyleColor(1);
}
