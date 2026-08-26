#include "UI/Widgets/UITextBox.hpp"
#include "Core/PreRequisites.h"
#include "Input/ShortcutManager.hpp"
#include "Utils/TimeGUI.hpp"

UITextBox::UITextBox(const TEString &placeholder, const TEString &id) : UIWidget(id), m_Placeholder(placeholder)
{
    m_Text.Reserve(256);
    SetupShortcutListener();
}

UITextBox::~UITextBox() { RemoveShortcutListener(); }

void UITextBox::SetupShortcutListener()
{
    m_ListenerName = "UITextBox_" + m_ID;
    ShortcutManager::AddListener(m_ListenerName,
                                 [this](const TEString &shortcutId) -> bool
                                 {
                                     if (!m_IsFocused)
                                         return false;

                                     if (shortcutId == "Editor_SelectAll")
                                     {
                                         SelectAll();
                                         return true;
                                     }
                                     else if (shortcutId == "Editor_Copy")
                                     {
                                         Copy();
                                         return true;
                                     }
                                     else if (shortcutId == "Editor_Cut")
                                     {
                                         Cut();
                                         return true;
                                     }
                                     else if (shortcutId == "Editor_Paste")
                                     {
                                         Paste();
                                         return true;
                                     }
                                     else if (shortcutId == "Editor_Undo")
                                     {
                                         Undo();
                                         return true;
                                     }
                                     else if (shortcutId == "Editor_Redo")
                                     {
                                         Redo();
                                         return true;
                                     }
                                     return false;
                                 });
}

void UITextBox::RemoveShortcutListener() { ShortcutManager::RemoveListener(m_ListenerName); }

void UITextBox::SetText(const TEString &text)
{
    if (m_Text != text)
    {
        m_Text = text;
        if (m_Text.Capacity() < 256)
            m_Text.Reserve(256);
        if (OnTextChanged)
            OnTextChanged(m_Text);
    }
}

void UITextBox::Clear() { SetText(""); }

void UITextBox::Focus() { m_FocusRequested = true; }

void UITextBox::SelectAll() { m_SelectAllRequested = true; }

void UITextBox::Copy()
{
    if (!m_Text.empty())
    {
        TimeGUI::SetClipboardText(m_Text);
    }
}

void UITextBox::Cut()
{
    if (!m_ReadOnly && !m_Text.empty())
    {
        TimeGUI::SetClipboardText(m_Text);
        Clear();
    }
}

void UITextBox::Paste()
{
    if (!m_ReadOnly)
    {
        TEString clip = TimeGUI::GetClipboardText();
        if (!clip.empty())
        {
            SetText(clip);
        }
    }
}

void UITextBox::Undo()
{
    // Handled by native text input layer
}

void UITextBox::Redo()
{
    // Handled by native text input layer
}

void UITextBox::DrawSelf()
{
    if (m_Text.Capacity() < 256)
        m_Text.Reserve(256);

    if (m_FocusRequested)
    {
        TimeGUI::SetKeyboardFocusHere();
        m_FocusRequested = false;
    }

    if (m_Size.x > 0)
        TimeGUI::SetNextItemWidth(m_Size.x);

    TimeGUIInputTextFlags flags = TimeGUIInputTextFlags_EnterReturnsTrue;
    if (m_ReadOnly)
        flags |= TimeGUIInputTextFlags_ReadOnly;
    if (m_Mode == ETextBoxMode::Password)
        flags |= TimeGUIInputTextFlags_Password;
    if (m_SelectAllRequested)
    {
        flags |= TimeGUIInputTextFlags_AutoSelectAll;
        m_SelectAllRequested = false;
    }

    bool entered = false;
    TEString prevText = m_Text;

    if (m_Mode == ETextBoxMode::MultiLine)
    {
        TEVector2 multilineSize = TEVector2(m_Size.x > 0 ? m_Size.x : 0, m_Size.y > 0 ? m_Size.y : 100.0f);
        entered = TimeGUI::InputTextMultiline(m_ID, m_Text, multilineSize, flags);
    }
    else if (!m_Placeholder.empty())
    {
        entered = TimeGUI::InputTextWithHint(m_ID, m_Placeholder, m_Text, flags);
    }
    else
    {
        entered = TimeGUI::InputText(m_ID, m_Text, flags);
    }

    // Focus state tracking
    m_IsFocused = TimeGUI::IsItemActive();
    if (m_IsFocused && !m_WasFocused)
    {
        if (OnFocusGained)
            OnFocusGained();
    }
    else if (!m_IsFocused && m_WasFocused)
    {
        if (OnFocusLost)
            OnFocusLost();
        m_Text.AutoCompact();
    }
    m_WasFocused = m_IsFocused;

    // Text mutation & Enter event delegates
    if (m_Text != prevText && OnTextChanged)
    {
        OnTextChanged(m_Text);
    }

    if (entered && OnEnterPressed)
    {
        OnEnterPressed(m_Text);
    }
}
