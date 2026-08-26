#pragma once
#include "UI/UIWidget.hpp"

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
    UIBorder(const TEVector4 &bgColor = {0.08f, 0.09f, 0.11f, 0.85f},
             const TEVector4 &borderColor = {0.16f, 0.18f, 0.22f, 1.0f}, float rounding = 6.0f,
             const TEString &id = "##UIBorder")
        : UIWidget(id), m_BgColor(bgColor), m_BorderColor(borderColor), m_Rounding(rounding)
    {
    }

    void SetPadding(const TEVector2 &pad) { m_Padding = pad; }

    void Draw() override
    {
        if (!m_IsVisible)
            return;
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, m_Rounding);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowBorderSize, 1.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, m_Padding);

        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, m_BgColor);
        TimeGUI::PushStyleColor(TimeGUICol_Border, m_BorderColor);

        TimeGUI::BeginChild(m_ID, m_Size, true);
        DrawSelf();
        for (auto &child : m_Children)
        {
            if (child && child->IsVisible())
                child->Draw();
        }
        TimeGUI::EndChild();

        TimeGUI::PopStyleColor(2);
        TimeGUI::PopStyleVar(3);
    }

protected:
    void DrawSelf() override {}

private:
    TEVector4 m_BgColor;
    TEVector4 m_BorderColor;
    TEVector2 m_Padding = {8.0f, 8.0f};
    float m_Rounding;
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
// UIButton - Modular Action Button
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

protected:
    void DrawSelf() override
    {
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
    }

private:
    TEString m_Label;
};

// =========================================================================
// UISlider - Float / Value control
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

    std::function<void(float)> OnValueChanged;

protected:
    void DrawSelf() override
    {
        if (m_Size.x > 0.0f)
            TimeGUI::SetNextItemWidth(m_Size.x);

        if (TimeGUI::SliderFloat(m_Label + m_ID, &m_Value, m_Min, m_Max))
        {
            if (OnValueChanged)
                OnValueChanged(m_Value);
        }
    }

private:
    TEString m_Label;
    float m_Value = 0.0f;
    float m_Min = 0.0f;
    float m_Max = 1.0f;
};
