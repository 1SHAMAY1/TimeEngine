#pragma once
#include "UIWidget.hpp"

// =========================================================================
// UIScrollBox - Scrollable container
// =========================================================================
class TE_API UIScrollBox : public UIWidget
{
public:
    UIScrollBox(const TEVector2 &size = {0.0f, 0.0f}, const TEString &id = "##UIScrollBox") : UIWidget(id)
    {
        m_Size = size;
    }

    void Draw() override
    {
        if (!m_IsVisible)
            return;
        TimeGUI::BeginChild(m_ID, m_Size, false, TimeGUIWindowFlags_AlwaysVerticalScrollbar);
        DrawSelf();
        for (auto &child : m_Children)
        {
            if (child && child->IsVisible())
                child->Draw();
        }
        TimeGUI::EndChild();
    }

protected:
    void DrawSelf() override {}
};

// =========================================================================
// UIBorder - Glass styled card container
// =========================================================================
class TE_API UIBorder : public UIWidget
{
public:
    UIBorder(const TEOption<TEVector4> &bgColor = {}, const TEOption<TEVector4> &borderColor = {},
             const TEOption<float> &rounding = {}, const TEString &id = "##UIBorder")
        : UIWidget(id), m_BgColor(bgColor), m_BorderColor(borderColor), m_Rounding(rounding)
    {
    }

    void SetCustomBackgroundColor(const TEOption<TEVector4> &col) { m_BgColor = col; }
    const TEOption<TEVector4> &GetCustomBackgroundColor() const { return m_BgColor; }

    void SetCustomBorderColor(const TEOption<TEVector4> &col) { m_BorderColor = col; }
    const TEOption<TEVector4> &GetCustomBorderColor() const { return m_BorderColor; }

    void SetCustomRounding(const TEOption<float> &rounding) { m_Rounding = rounding; }
    const TEOption<float> &GetCustomRounding() const { return m_Rounding; }

    void SetPadding(const TEVector2 &pad) { m_Padding = pad; }
    const TEVector2 &GetPadding() const { return m_Padding; }

    void Draw() override
    {
        if (!m_IsVisible)
            return;

        int pushedVars = 0;
        int pushedCols = 0;

        if (m_Rounding.has_value())
        {
            TimeGUI::PushStyleVar(TimeGUIStyleVar_ChildRounding, m_Rounding.value());
            pushedVars++;
        }
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, m_Padding);
        pushedVars++;

        if (m_BgColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_ChildBg, m_BgColor.value());
            pushedCols++;
        }
        if (m_BorderColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_Border, m_BorderColor.value());
            pushedCols++;
        }

        TimeGUI::BeginChild(m_ID, m_Size, true);
        DrawSelf();
        for (auto &child : m_Children)
        {
            if (child && child->IsVisible())
                child->Draw();
        }
        TimeGUI::EndChild();

        if (pushedCols > 0)
            TimeGUI::PopStyleColor(pushedCols);
        if (pushedVars > 0)
            TimeGUI::PopStyleVar(pushedVars);
    }

protected:
    void DrawSelf() override {}

private:
    TEOption<TEVector4> m_BgColor;
    TEOption<TEVector4> m_BorderColor;
    TEOption<float> m_Rounding;
    TEVector2 m_Padding = {8.0f, 8.0f};
};

// =========================================================================
// UISizeBox - Enforces explicit dimensions
// =========================================================================
class TE_API UISizeBox : public UIWidget
{
public:
    UISizeBox(const TEVector2 &explicitSize, const TEString &id = "##UISizeBox") : UIWidget(id)
    {
        m_Size = explicitSize;
    }

    void Draw() override
    {
        if (!m_IsVisible)
            return;
        TimeGUI::BeginChild(m_ID, m_Size, false);
        DrawSelf();
        for (auto &child : m_Children)
        {
            if (child && child->IsVisible())
                child->Draw();
        }
        TimeGUI::EndChild();
    }

protected:
    void DrawSelf() override {}
};

// =========================================================================
// UIButton - Modular Action Button with Per-Instance Customization
// =========================================================================
class TE_API UIButton : public UIWidget
{
public:
    UIButton(const TEString &label, const TEVector2 &size = {0.0f, 0.0f}, const TEString &id = "")
        : UIWidget(id.empty() ? label : id), m_Label(label)
    {
        m_Size = size;
    }

    void SetLabel(const TEString &label) { m_Label = label; }
    const TEString &GetLabel() const { return m_Label; }

    // Per-Instance Style Overrides
    void SetCustomNormalColor(const TEOption<TEVector4> &col) { m_CustomNormalColor = col; }
    const TEOption<TEVector4> &GetCustomNormalColor() const { return m_CustomNormalColor; }

    void SetCustomHoveredColor(const TEOption<TEVector4> &col) { m_CustomHoveredColor = col; }
    const TEOption<TEVector4> &GetCustomHoveredColor() const { return m_CustomHoveredColor; }

    void SetCustomActiveColor(const TEOption<TEVector4> &col) { m_CustomActiveColor = col; }
    const TEOption<TEVector4> &GetCustomActiveColor() const { return m_CustomActiveColor; }

    void SetCustomTextColor(const TEOption<TEVector4> &col) { m_CustomTextColor = col; }
    const TEOption<TEVector4> &GetCustomTextColor() const { return m_CustomTextColor; }

    void SetCustomRounding(const TEOption<float> &rounding) { m_CustomRounding = rounding; }
    const TEOption<float> &GetCustomRounding() const { return m_CustomRounding; }

protected:
    void DrawSelf() override
    {
        int pushedCols = 0;
        int pushedVars = 0;

        if (m_CustomNormalColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, m_CustomNormalColor.value());
            pushedCols++;
        }
        if (m_CustomHoveredColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, m_CustomHoveredColor.value());
            pushedCols++;
        }
        if (m_CustomActiveColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, m_CustomActiveColor.value());
            pushedCols++;
        }
        if (m_CustomTextColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_Text, m_CustomTextColor.value());
            pushedCols++;
        }
        if (m_CustomRounding.has_value())
        {
            TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, m_CustomRounding.value());
            pushedVars++;
        }

        if (TimeGUI::Button(m_Label + m_ID, m_Size))
        {
            if (OnClicked)
                OnClicked(this);
        }
        if (TimeGUI::IsItemHovered())
        {
            if (OnHovered)
                OnHovered(this);
        }

        if (pushedCols > 0)
            TimeGUI::PopStyleColor(pushedCols);
        if (pushedVars > 0)
            TimeGUI::PopStyleVar(pushedVars);
    }

private:
    TEString m_Label;
    TEOption<TEVector4> m_CustomNormalColor;
    TEOption<TEVector4> m_CustomHoveredColor;
    TEOption<TEVector4> m_CustomActiveColor;
    TEOption<TEVector4> m_CustomTextColor;
    TEOption<float> m_CustomRounding;
};

// =========================================================================
// UISlider - Float / Value control with Per-Instance Customization
// =========================================================================
class TE_API UISlider : public UIWidget
{
public:
    UISlider(const TEString &label, float minVal = 0.0f, float maxVal = 1.0f, const TEString &id = "")
        : UIWidget(id.empty() ? label : id), m_Label(label), m_Min(minVal), m_Max(maxVal)
    {
    }

    void SetValue(float val) { m_Value = val; }
    float GetValue() const { return m_Value; }

    void SetRange(float minVal, float maxVal)
    {
        m_Min = minVal;
        m_Max = maxVal;
    }

    // Per-Instance Style Overrides
    void SetCustomFrameBg(const TEOption<TEVector4> &col) { m_CustomFrameBg = col; }
    const TEOption<TEVector4> &GetCustomFrameBg() const { return m_CustomFrameBg; }

    void SetCustomGrabColor(const TEOption<TEVector4> &col) { m_CustomGrabColor = col; }
    const TEOption<TEVector4> &GetCustomGrabColor() const { return m_CustomGrabColor; }

    void SetCustomGrabActiveColor(const TEOption<TEVector4> &col) { m_CustomGrabActiveColor = col; }
    const TEOption<TEVector4> &GetCustomGrabActiveColor() const { return m_CustomGrabActiveColor; }

    void SetCustomTextColor(const TEOption<TEVector4> &col) { m_CustomTextColor = col; }
    const TEOption<TEVector4> &GetCustomTextColor() const { return m_CustomTextColor; }

    void SetCustomRounding(const TEOption<float> &rounding) { m_CustomRounding = rounding; }
    const TEOption<float> &GetCustomRounding() const { return m_CustomRounding; }

    std::function<void(float)> OnValueChanged;

protected:
    void DrawSelf() override
    {
        int pushedCols = 0;
        int pushedVars = 0;

        if (m_CustomFrameBg.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_FrameBg, m_CustomFrameBg.value());
            pushedCols++;
        }
        if (m_CustomGrabColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_SliderGrab, m_CustomGrabColor.value());
            pushedCols++;
        }
        if (m_CustomGrabActiveColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_SliderGrabActive, m_CustomGrabActiveColor.value());
            pushedCols++;
        }
        if (m_CustomTextColor.has_value())
        {
            TimeGUI::PushStyleColor(TimeGUICol_Text, m_CustomTextColor.value());
            pushedCols++;
        }
        if (m_CustomRounding.has_value())
        {
            TimeGUI::PushStyleVar(TimeGUIStyleVar_GrabRounding, m_CustomRounding.value());
            pushedVars++;
        }

        if (m_Size.x > 0.0f)
            TimeGUI::SetNextItemWidth(m_Size.x);

        if (TimeGUI::SliderFloat(m_Label + m_ID, &m_Value, m_Min, m_Max))
        {
            if (OnValueChanged)
                OnValueChanged(m_Value);
        }

        if (pushedCols > 0)
            TimeGUI::PopStyleColor(pushedCols);
        if (pushedVars > 0)
            TimeGUI::PopStyleVar(pushedVars);
    }

private:
    TEString m_Label;
    float m_Value = 0.0f;
    float m_Min = 0.0f;
    float m_Max = 1.0f;

    TEOption<TEVector4> m_CustomFrameBg;
    TEOption<TEVector4> m_CustomGrabColor;
    TEOption<TEVector4> m_CustomGrabActiveColor;
    TEOption<TEVector4> m_CustomTextColor;
    TEOption<float> m_CustomRounding;
};
