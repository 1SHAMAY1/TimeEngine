#include "UI/Widgets/UIToggleSwitch.hpp"
#include "Core/PreRequisites.h"

UIToggleSwitch::UIToggleSwitch(bool initialState, const TEVector2 &size, const TEString &id)
    : UIWidget(id), m_IsChecked(initialState)
{
    m_Size = size;
}

void UIToggleSwitch::SetChecked(bool checked)
{
    if (m_IsChecked != checked)
    {
        m_IsChecked = checked;
        if (OnToggled)
            OnToggled(m_IsChecked);
    }
}

void UIToggleSwitch::Toggle() { SetChecked(!m_IsChecked); }

void UIToggleSwitch::DrawSelf()
{
    bool state = m_IsChecked;
    if (DrawToggle(m_ID, state, m_Size))
    {
        SetChecked(state);
    }
}

bool UIToggleSwitch::DrawToggle(const TEString &id, bool &checked, const TEVector2 &size)
{
    float switchWidth = size.x > 0.0f ? size.x : 52.0f;
    float switchHeight = size.y > 0.0f ? size.y : 28.0f;

    TEVector2 switchMin = TimeGUI::GetCursorScreenPos();
    TEVector2 switchMax = TEVector2(switchMin.x + switchWidth, switchMin.y + switchHeight);

    bool changed = false;
    if (TimeGUI::InvisibleButton(id.c_str(), TEVector2(switchWidth, switchHeight)))
    {
        checked = !checked;
        changed = true;
    }

    bool isHovered = TimeGUI::IsItemHovered();
    float radius = switchHeight * 0.5f;
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    // Track Background & Outline
    unsigned int trackBg = checked ? (isHovered ? 0xFF32C66D : 0xFF28B568) : (isHovered ? 0xFF3B485C : 0xFF283240);
    unsigned int trackBorder = checked ? 0xFF45E585 : 0xFF45556C;
    dl.AddRectFilled(switchMin, switchMax, trackBg, radius);
    dl.AddRect(switchMin, switchMax, trackBorder, radius, 0, 1.2f);

    // Circular Knob with Soft Drop Shadow
    float knobRadius = radius - 3.0f;
    float knobX = checked ? (switchMax.x - radius) : (switchMin.x + radius);
    float knobY = switchMin.y + radius;
    TEVector2 knobCenter = TEVector2(knobX, knobY);

    dl.AddCircleFilled(TEVector2(knobCenter.x, knobCenter.y + 1.0f), knobRadius, 0x44000000);
    dl.AddCircleFilled(knobCenter, knobRadius, checked ? 0xFFFFFFFF : 0xFFD0DCE8);

    return changed;
}
