#include "Core/PreRequisites.h"
#include "Layers/LogoLayer.hpp"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Plugin/PluginManager.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

LogoLayer::LogoLayer(const TEString &name) : Layer(name), m_ShouldClose(false)
{
    m_StatusText = "Discovering Engine Plugins...";
    PluginManager::StartAsyncLoading();
}

LogoLayer::~LogoLayer() {}

void LogoLayer::OnAttach() { TE_CORE_INFO("[LogoLayer] OnAttach called. Starting welcome animation."); }

void LogoLayer::OnWelcomeAnimationComplete()
{
    TE_CORE_INFO("[LogoLayer] OnWelcomeAnimationComplete fired.");
    m_ShouldBroadcast = true;
    m_ShouldClose = true;
}

void LogoLayer::OnTimeGUIRender()
{
    TimeGUI::TimeGUIIO io = TimeGUI::GetIO();
    float dt = io.DeltaTime > 0.0f ? io.DeltaTime : 0.016f;
    m_Time += dt;

    static float s_LastLogTime = 0.0f;
    bool shouldLogHeartbeat = (m_Time - s_LastLogTime) >= 1.0f;

    // ── Asynchronous Plugin Loading (Real-time queue draining) ────────────────
    if (!m_PluginsLoaded)
    {
        PluginProgressMessage progress;
        while (PluginManager::TryGetAsyncProgress(progress))
        {
            if (progress.TotalCount > 0)
            {
                m_TargetProgress = (float)progress.LoadedCount / (float)progress.TotalCount;
                m_StatusText = "Loading Plugin: " + progress.PluginName + " [" +
                               TEString::FromInt((int)progress.LoadedCount) + " of " +
                               TEString::FromInt((int)progress.TotalCount) + "]";
            }
            else
            {
                m_TargetProgress = 1.0f;
                m_StatusText = "Initializing Core Systems...";
            }

            if (progress.IsComplete)
            {
                m_PluginsLoaded = true;
                m_TargetProgress = 1.0f;
                m_StatusText = "Ready. Launching Workspace...";
                m_FinishTime = m_Time;
                TE_CORE_INFO("[LogoLayer] Async plugin loading complete! FinishTime: %.2fs", m_FinishTime);
                break;
            }
        }

        if (!m_PluginsLoaded && (PluginManager::IsAsyncLoadingComplete() || PluginManager::IsFullyLoaded()))
        {
            m_PluginsLoaded = true;
            m_TargetProgress = 1.0f;
            m_StatusText = "Ready. Launching Workspace...";
            m_FinishTime = m_Time;
            TE_CORE_INFO("[LogoLayer] PluginManager reports fully loaded! FinishTime: %.2fs", m_FinishTime);
        }
    }

    // Smoothly interpolate display progress towards target using engine Clamp and Lerp
    float t = Clamp(dt * 10.0f, 0.0f, 1.0f);
    m_DisplayProgress = Lerp(m_DisplayProgress, m_TargetProgress, t);
    m_DisplayProgress = Clamp(m_DisplayProgress, 0.0f, 1.0f);

    // Fade-in at start, fade-out at finish
    float alpha = 1.0f;
    if (m_Time < 0.3f)
    {
        alpha = m_Time / 0.3f;
    }
    else if (m_PluginsLoaded && (m_Time - m_FinishTime) > 0.4f)
    {
        float fadeElapsed = (m_Time - m_FinishTime) - 0.4f;
        alpha = Max(0.0f, 1.0f - (fadeElapsed / 0.3f));

        if (alpha <= 0.0f && !m_AnimationFinished)
        {
            m_AnimationFinished = true;
            TE_CORE_INFO("[LogoLayer] Welcome animation finished (Alpha <= 0.0). Triggering completion.");
            OnWelcomeAnimationComplete();
        }
    }

    if (shouldLogHeartbeat)
    {
        s_LastLogTime = m_Time;
        TE_CORE_INFO("[LogoLayer] Heartbeat | Time: %.2fs | Progress: %.1f%% | Alpha: %.2f | Status: %s", m_Time,
                     m_DisplayProgress * 100.0f, alpha, m_StatusText.c_str());
    }

    // ── Render Loading Screen Visuals ─────────────────────────────────────────
    TimeGUI::TimeGUIViewport viewport = TimeGUI::GetMainViewport();
    TEVector2 screenPos = viewport.Pos;
    TEVector2 screenSize = viewport.Size;
    TEVector2 center = screenPos + screenSize * 0.5f;

    static bool s_LoggedViewport = false;
    if (!s_LoggedViewport && screenSize.x > 0.0f)
    {
        TE_CORE_INFO("[LogoLayer] Main Viewport: (%.1f, %.1f) - Size: (%.1f, %.1f) - Center: (%.1f, %.1f)", screenPos.x,
                     screenPos.y, screenSize.x, screenSize.y, center.x, center.y);
        s_LoggedViewport = true;
    }

    TimeGUI::TimeGUIDrawList drawList = TimeGUI::GetBackgroundDrawList();

    // 1. Background Fill (#0B0E14)
    drawList.AddRectFilled(screenPos, screenPos + screenSize, TimeGUI::GetColorU32(TEColor(0.04f, 0.05f, 0.08f, 1.0f)));

    // 2. Animated Pulsing Neon Logo (Centered)
    float animTime = m_Time;
    float blueRingGlowPulse = 0.5f + 0.5f * sinf(animTime * 2.5f);
    float redHandFade = 0.5f + 0.5f * sinf(animTime * 3.2f);
    float whiteHandFade = 0.5f + 0.5f * sinf(animTime * 2.8f + 1.0f);
    float grayHandFade = 0.5f + 0.5f * sinf(animTime * 2.2f + 2.0f);

    float radius = 56.0f;

    // Glowing Neon Rings
    TimeGUIColor32 softBlueGlow =
        TimeGUI::GetColorU32(TEColor(0.0f, 0.65f, 1.0f, alpha * (0.25f + 0.35f * blueRingGlowPulse)));
    TimeGUIColor32 brightCyanRing =
        TimeGUI::GetColorU32(TEColor(0.0f, 0.90f, 1.0f, alpha * (0.75f + 0.25f * blueRingGlowPulse)));

    drawList.AddCircle(center, radius + 4.0f, softBlueGlow, 64, 6.0f);
    drawList.AddCircle(center, radius, brightCyanRing, 64, 2.2f);

    // Minimalist Clock Hands
    TimeGUIColor32 redHandColor =
        TimeGUI::GetColorU32(TEColor(0.95f, 0.25f, 0.25f, alpha * (0.4f + 0.6f * redHandFade)));
    TimeGUIColor32 whiteHandColor =
        TimeGUI::GetColorU32(TEColor(1.0f, 1.0f, 1.0f, alpha * (0.4f + 0.6f * whiteHandFade)));
    TimeGUIColor32 grayHandColor =
        TimeGUI::GetColorU32(TEColor(0.6f, 0.68f, 0.78f, alpha * (0.4f + 0.6f * grayHandFade)));
    TimeGUIColor32 centerPinColor = TimeGUI::GetColorU32(TEColor(1.0f, 1.0f, 1.0f, alpha));

    drawList.AddLine(center, center + TEVector2(radius * 0.70f, 0.0f), redHandColor, 2.2f);
    drawList.AddLine(center, center + TEVector2(-radius * 0.70f, 0.0f), whiteHandColor, 2.2f);
    drawList.AddLine(center, center + TEVector2(0.0f, radius * 0.55f), grayHandColor, 2.6f);
    drawList.AddCircleFilled(center, 3.0f, centerPinColor, 32);

    // 3. Status Text & Percentage at Bottom Extreme Points
    float paddingX = 24.0f;
    float textY = screenPos.y + screenSize.y - 32.0f;
    TimeGUIColor32 statusCol = TimeGUI::GetColorU32(TEColor(0.60f, 0.72f, 0.88f, alpha));

    // Left Extreme Point: Current Loading Status & Plugin Name
    drawList.AddText(TEVector2(screenPos.x + paddingX, textY), statusCol, m_StatusText.c_str());

    // Right Extreme Point: Percentage
    int pct = (int)(m_DisplayProgress * 100.0f);
    TEString pctStr = TEString::FromInt(pct) + "%";
    drawList.AddText(TEVector2(screenPos.x + screenSize.x - paddingX - 36.0f, textY), statusCol, pctStr.c_str());

    // 4. Edge-to-Edge Progress Bar at Very Bottom
    float barHeight = 4.0f;
    TEVector2 barMin = TEVector2(screenPos.x, screenPos.y + screenSize.y - barHeight);
    TEVector2 barMax = TEVector2(screenPos.x + screenSize.x, screenPos.y + screenSize.y);

    // Track
    drawList.AddRectFilled(barMin, barMax, TimeGUI::GetColorU32(TEColor(0.08f, 0.11f, 0.16f, alpha * 0.9f)));

    // Fill
    float filledW = screenSize.x * Clamp(m_DisplayProgress, 0.0f, 1.0f);
    if (filledW > 0.0f)
    {
        drawList.AddRectFilled(barMin, TEVector2(barMin.x + filledW, barMax.y),
                               TimeGUI::GetColorU32(TEColor(0.0f, 0.85f, 1.0f, alpha)));
        // Subtle top glow accent line on the progress bar
        drawList.AddLine(TEVector2(barMin.x, barMin.y), TEVector2(barMin.x + filledW, barMin.y),
                         TimeGUI::GetColorU32(TEColor(0.3f, 0.95f, 1.0f, alpha * 0.7f)), 1.5f);
    }

    // ── Broadcast & Layer Removal ─────────────────────────────────────────────
    if (m_ShouldBroadcast)
    {
        m_ShouldBroadcast = false;
        TE_CORE_INFO("[LogoLayer] Broadcasting LogoFinishedDelegate...");
        LogoFinishedDelegate.Broadcast();
    }

    if (m_ShouldClose && !m_ShouldBroadcast && !m_IsBeingRemoved)
    {
        TE_CORE_INFO("[LogoLayer] Startup Loading finished. Marking LogoLayer for removal.");
        m_IsBeingRemoved = true;
        Application::Get().MarkLayerForRemoval(shared_from_this());
    }
}

void LogoLayer::OnDetach()
{
    TE_CORE_INFO("[LogoLayer] OnDetach called. Cleaning up.");
    m_Time = 0.0f;
    m_DisplayProgress = 0.0f;
    m_TargetProgress = 0.0f;
    m_PluginsLoaded = false;
    m_AnimationStarted = false;
    m_AnimationFinished = false;
    m_ShouldClose = false;
    m_ShouldBroadcast = false;
    m_IsBeingRemoved = false;
}
