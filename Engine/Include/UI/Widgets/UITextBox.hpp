#pragma once

#include "UI/UIWidget.hpp"
#include <functional>

enum class ETextBoxMode
{
    SingleLine,
    MultiLine,
    Password
};

class TE_API UITextBox : public UIWidget
{
public:
    UITextBox(const TEString &placeholder = "", const TEString &id = "##UITextBox");
    virtual ~UITextBox();

    void DrawSelf() override;

    // Value Management
    const TEString &GetText() const { return m_Text; }
    void SetText(const TEString &text);
    void Clear();

    const TEString &GetPlaceholder() const { return m_Placeholder; }
    void SetPlaceholder(const TEString &placeholder) { m_Placeholder = placeholder; }

    // Configuration
    void SetMode(ETextBoxMode mode) { m_Mode = mode; }
    ETextBoxMode GetMode() const { return m_Mode; }

    void SetReadOnly(bool readOnly) { m_ReadOnly = readOnly; }
    bool IsReadOnly() const { return m_ReadOnly; }

    void Focus();
    bool IsFocused() const { return m_IsFocused; }

    // Caret & Selection
    void SelectAll();
    void Copy();
    void Cut();
    void Paste();
    void Undo();
    void Redo();

    // Event Delegates
    std::function<void(const TEString &)> OnTextChanged;
    std::function<void(const TEString &)> OnEnterPressed;
    std::function<void()> OnFocusGained;
    std::function<void()> OnFocusLost;

private:
    void SetupShortcutListener();
    void RemoveShortcutListener();
    void DrawWhitePulseStrip();

    TEString m_Text;
    TEString m_Placeholder;
    ETextBoxMode m_Mode = ETextBoxMode::SingleLine;
    bool m_ReadOnly = false;
    bool m_FocusRequested = false;
    bool m_IsFocused = false;
    bool m_WasFocused = false;
    bool m_SelectAllRequested = false;

    TEString m_ListenerName;
};
