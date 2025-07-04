#include "Layers/LogoLayer.hpp"
#include "imgui_internal.h"
#include "imgui.h"
#include "Utility/MathUtils.hpp"
#include "Core/Application.h"

namespace TE {

    using namespace Math;

    // Constructor
    LogoLayer::LogoLayer(const std::string& name)
        : Layer(name), m_ShouldClose(false)
    {
    }

    // Destructor
    LogoLayer::~LogoLayer() {}

    // Called when welcome animation is done
    void LogoLayer::OnWelcomeAnimationComplete()
    {
        LogoFinishedDelegate.Broadcast();

        m_ShouldClose = true; // Request the application to remove this layer
    }


    // Render the logo animation using ImGui draw lists
    void LogoLayer::OnImGuiRender()
    {
        ImGuiIO& io = ImGui::GetIO();
        m_Time += io.DeltaTime;

        // Start animation after a short delay
        if (!m_AnimationStarted && m_Time >= 1.0f)
        {
            m_AnimationStarted = true;
            m_AnimationStartTime = m_Time;
            m_CharIndex = 0;
            m_DisplayText.clear();
        }

        // Exit if not animating or finished
        if (!m_AnimationStarted || m_AnimationFinished)
        {
            if (m_ShouldClose)
                Application::Get().PopLayer(this);
            return;
        }

        // Setup screen geometry and center
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 screenPos = viewport->Pos;
        ImVec2 screenSize = viewport->Size;
        ImVec2 center = screenPos + screenSize * 0.5f;
        center.y -= screenSize.y * 0.1f;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList(viewport);

        // === Background Fill ===
        drawList->AddRectFilled(screenPos, screenPos + screenSize, IM_COL32(0, 0, 0, 255));

        // === Animate Text Reveal ===
        size_t targetCount = static_cast<size_t>((m_Time - m_AnimationStartTime) / m_LetterInterval);
        if (targetCount > m_CharIndex && m_CharIndex < m_FullText.length())
        {
            m_CharIndex++;
            m_DisplayText = m_FullText.substr(0, m_CharIndex);
        }

        // === Logo Rendering ===
        DrawTimeEngineLogo(center, 40.0f, drawList, m_Time);

        // === Animated Text Rendering ===
        ImFont* font = ImGui::GetFont();
        float fontSize = 24.0f;
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, -1.0f, m_DisplayText.c_str());
        ImVec2 textPos = center + ImVec2(-textSize.x * 0.5f, 55.0f);
        drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, 255), m_DisplayText.c_str());

        // === Animation Completion Check ===
        if (m_CharIndex == m_FullText.length() &&
            (m_Time - m_AnimationStartTime) >= (m_FullText.length() * m_LetterInterval + m_TextClearDelay))
        {
            m_AnimationFinished = true;
            m_DisplayText.clear();
            OnWelcomeAnimationComplete();
        }

        // Layer removal check
        if (m_ShouldClose)
        {
            Application::Get().PopLayer(this);
        }
    }

    // Reset layer state on removal
    void LogoLayer::OnDetach()
    {
        m_Time = 0.0f;
        m_AnimationStartTime = 0.0f;
        m_CharIndex = 0;
        m_DisplayText.clear();
        m_AnimationStarted = false;
        m_AnimationFinished = false;
        m_ShouldClose = false;
    }

    // === Draw the animated TimeEngine Logo ===
    void LogoLayer::DrawTimeEngineLogo(const ImVec2& center, float radius, ImDrawList* drawList, float time)
    {
        const float pi = 3.1415926f;
        const ImU32 color       = IM_COL32(255, 255, 255, 255);
        const ImU32 faintColor  = IM_COL32(255, 255, 255, 60);
        const ImU32 strongColor = IM_COL32(255, 255, 255, 200);

        // === Static Clock Face Circle ===
        drawList->AddCircle(center, radius, color, 64, 2.5f);

        // === Hour Tick Marks ===
        for (int i = 0; i < 12; ++i)
        {
            float angle = i * (2.0f * pi / 12);
            ImVec2 dir = { cosf(angle), sinf(angle) };
            ImVec2 inner = center + dir * (radius - 4.0f);
            ImVec2 outer = center + dir * radius;
            drawList->AddLine(inner, outer, color, 1.8f);
        }

        // === Rotating Gear Arcs (Separated) ===
        const int gearTeeth = 12;
        float gearRotation = -time * 0.5f;
        float arcSpacing   = 50.0f;
        float arcThickness = 12.0f;
        float arcRadiusInner = radius + arcSpacing;
        float arcRadiusOuter = arcRadiusInner + arcThickness;

        for (int i = 0; i < gearTeeth; ++i)
        {
            float startAngle = i * (2.0f * pi / gearTeeth) + gearRotation;
            float endAngle   = startAngle + (2.0f * pi / gearTeeth) * 0.6f;

            const int arcSegments = 8;
            for (int j = 0; j < arcSegments; ++j)
            {
                float t0 = startAngle + (endAngle - startAngle) * (j / (float)arcSegments);
                float t1 = startAngle + (endAngle - startAngle) * ((j + 1) / (float)arcSegments);

                ImVec2 p0_outer = center + ImVec2(cosf(t0), sinf(t0)) * arcRadiusOuter;
                ImVec2 p1_outer = center + ImVec2(cosf(t1), sinf(t1)) * arcRadiusOuter;
                ImVec2 p0_inner = center + ImVec2(cosf(t0), sinf(t0)) * arcRadiusInner;
                ImVec2 p1_inner = center + ImVec2(cosf(t1), sinf(t1)) * arcRadiusInner;

                drawList->AddQuadFilled(p0_inner, p1_inner, p1_outer, p0_outer, faintColor);
            }
        }

        // === Clock Hands (Fast Spinning) ===
        float fastTime = time * 100.0f;
        float seconds = fmod(fastTime, 60.0f);
        float minutes = fmod(fastTime * 5 / 60.0f, 60.0f);
        float hours   = fmod(fastTime * 10 / 3600.0f, 12.0f);

        float secondAngle = -pi / 2.0f + seconds * (2.0f * pi / 60.0f);
        float minuteAngle = -pi / 2.0f + minutes * (2.0f * pi / 60.0f);
        float hourAngle   = -pi / 2.0f + hours * (2.0f * pi / 12.0f);

        ImVec2 secDir = { cosf(secondAngle), sinf(secondAngle) };
        ImVec2 minDir = { cosf(minuteAngle), sinf(minuteAngle) };
        ImVec2 hrDir  = { cosf(hourAngle), sinf(hourAngle) };

        drawList->AddLine(center, center + secDir * (radius - 5.0f), IM_COL32(255, 50, 50, 200), 1.5f);     // Second hand
        drawList->AddLine(center, center + minDir * (radius - 10.0f), strongColor, 2.5f);                  // Minute hand
        drawList->AddLine(center, center + hrDir * (radius - 20.0f), color, 3.5f);                         // Hour hand

        // === Clock Center Pin ===
        drawList->AddCircleFilled(center, 3.0f, color);
    }

}
