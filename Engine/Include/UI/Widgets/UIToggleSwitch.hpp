#pragma once
#include "UI/UIWidget.hpp"
#include <functional>

class TE_API UIToggleSwitch : public UIWidget
{
public:
    UIToggleSwitch(bool initialState = false, const TEVector2 &size = TEVector2(52.0f, 28.0f),
                   const TEString &id = "##UIToggleSwitch");

    void DrawSelf() override;

    bool IsChecked() const { return m_IsChecked; }
    void SetChecked(bool checked);
    void Toggle();

    // Static immediate-mode helper for anywhere in TimeGUI
    static bool DrawToggle(const TEString &id, bool &checked, const TEVector2 &size = TEVector2(52.0f, 28.0f));

    std::function<void(bool)> OnToggled;

private:
    bool m_IsChecked = false;
};
