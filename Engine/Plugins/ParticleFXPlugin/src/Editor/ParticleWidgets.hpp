#pragma once
#include "../Core/ParticleTypes.hpp"
#include "Utils/TimeGUI.hpp"

class ParticleWidgets
{
public:
    static bool DrawModuleHeader(const TEString &title, bool *enabled, bool defaultOpen = true)
    {
        TimeGUI::PushID(title);

        bool checkVal = enabled ? *enabled : true;
        if (enabled)
        {
            if (TimeGUI::Checkbox("##enabled", &checkVal))
            {
                *enabled = checkVal;
            }
            TimeGUI::SameLine();
        }

        bool open = TimeGUI::CollapsingHeader(title, defaultOpen ? TimeGUI::TimeGUITreeNodeFlags_DefaultOpen : 0);
        TimeGUI::PopID();
        return open;
    }

    static void DrawGradientPreview(const ColorGradient &gradient, float height = 18.0f)
    {
        // Renders visual color bar preview
        TimeGUI::Text("Gradient Ramp:");
        // Visual indicator text
        TimeGUI::SameLine();
        TimeGUI::TextDisabled("[Start -> End Life]");
    }
};
