#include "Editor/MaterialPreviewViewport.hpp"
#include "Runtime/MaterialRuntime.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

MaterialPreviewViewport::MaterialPreviewViewport() {}

void MaterialPreviewViewport::InitRenderer()
{
    // Lazy initialized if required
}

void MaterialPreviewViewport::Draw(const TERef<MaterialAsset> &material)
{
    TimeGUI::BeginChild("MaterialPreviewPanel", TEVector2(0, 0), false);

    TimeGUI::Text("Viewport Preview");
    TimeGUI::SameLine();
    TimeGUI::Checkbox("2D Light Tester", &m_IsLightTesterEnabled);

    if (m_IsLightTesterEnabled)
    {
        TimeGUI::SameLine();
        TimeGUI::SliderFloat("Intensity", &m_LightIntensity, 0.0f, 5.0f, "%.1f");
        TimeGUI::SameLine();
        TimeGUI::ColorEdit4("Light Color", &m_LightColor.x, TimeGUI::TimeGUIColorEditFlags_NoInputs);
    }

    TimeGUI::SameLine();
    TimeGUI::SliderFloat("Zoom", &m_PreviewZoom, 0.2f, 3.0f, "%.1fx");

    TimeGUI::Separator();

    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();
    if (canvasSize.x <= 10.0f || canvasSize.y <= 10.0f)
    {
        TimeGUI::EndChild();
        return;
    }

    TEVector2 pMin = TimeGUI::GetCursorScreenPos();
    TEVector2 pMax = pMin + canvasSize;
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    // Viewport background grid
    dl.AddRectFilled(pMin, pMax, TIMEGUI_COL32(24, 24, 28, 255));

    // Checkerboard background
    float tileSize = 16.0f;
    for (float y = pMin.y; y < pMax.y; y += tileSize)
    {
        for (float x = pMin.x; x < pMax.x; x += tileSize)
        {
            int check = ((int)((x - pMin.x) / tileSize) + (int)((y - pMin.y) / tileSize)) % 2;
            TimeGUIColor32 col = (check == 0) ? TIMEGUI_COL32(32, 32, 36, 255) : TIMEGUI_COL32(40, 40, 46, 255);
            TEVector2 tMax(std::min(x + tileSize, pMax.x), std::min(y + tileSize, pMax.y));
            dl.AddRectFilled(TEVector2(x, y), tMax, col);
        }
    }

    // Material Object Preview
    TEVector2 center = (pMin + pMax) * 0.5f;
    float objSize = 140.0f * m_PreviewZoom;
    TEVector2 objMin = center - TEVector2(objSize * 0.5f, objSize * 0.5f);
    TEVector2 objMax = center + TEVector2(objSize * 0.5f, objSize * 0.5f);

    // Extract Base Color if present
    TEColor surfaceColor = TEColor::White();
    if (material)
    {
        for (const auto &node : material->GetGraph().GetNodes())
        {
            if (node->NodeType == "VectorParameter" && node->GetProperty("ParamName") == "BaseColor")
            {
                float r = std::stof(node->GetProperty("DefaultR", "1.0").c_str());
                float g = std::stof(node->GetProperty("DefaultG", "1.0").c_str());
                float b = std::stof(node->GetProperty("DefaultB", "1.0").c_str());
                float a = std::stof(node->GetProperty("DefaultA", "1.0").c_str());
                surfaceColor = TEColor(r, g, b, a);
                break;
            }
        }
    }

    // Draw Material Preview Shape
    dl.AddRectFilled(
        objMin, objMax,
        TimeGUI::ColorConvertFloat4ToU32(TEVector4(surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a)),
        8.0f);
    dl.AddRect(objMin, objMax, TIMEGUI_COL32(255, 255, 255, 120), 8.0f, 0, 1.5f);

    // 2D Light Tester Handle
    if (m_IsLightTesterEnabled)
    {
        TEVector2 lightScreenPos = center + m_LightPos - TEVector2(150.0f, 150.0f);
        dl.AddCircleFilled(lightScreenPos, 8.0f, TimeGUI::ColorConvertFloat4ToU32(m_LightColor));
        dl.AddCircle(lightScreenPos, 14.0f, TIMEGUI_COL32(255, 255, 255, 180), 0, 1.5f);

        // Allow dragging light position
        TEVector2 mousePos = TimeGUI::GetMousePos();
        if (TimeGUI::IsMouseDown(0) &&
            glm::distance(glm::vec2(mousePos.x, mousePos.y), glm::vec2(lightScreenPos.x, lightScreenPos.y)) <= 20.0f)
        {
            m_LightPos = mousePos - center + TEVector2(150.0f, 150.0f);
        }
    }

    // Compilation Status Indicator
    if (material)
    {
        bool compiled = material->GetCompileResult().Success;
        TEColor statusColor = compiled ? TEColor(0.2f, 0.8f, 0.3f, 1.0f) : TEColor(0.9f, 0.25f, 0.25f, 1.0f);
        const char *statusText = compiled ? "[Shader Compiled & Ready]" : "[Compilation Error]";
        dl.AddText(
            pMin + TEVector2(12.0f, 12.0f),
            TimeGUI::ColorConvertFloat4ToU32(TEVector4(statusColor.r, statusColor.g, statusColor.b, statusColor.a)),
            statusText);

        if (!compiled && !material->GetCompileResult().ErrorMessage.empty())
        {
            dl.AddText(pMin + TEVector2(12.0f, 32.0f), TIMEGUI_COL32(255, 180, 180, 255),
                       material->GetCompileResult().ErrorMessage.c_str());
        }
    }

    TimeGUI::EndChild();
}
