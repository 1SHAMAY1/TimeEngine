#pragma once

#include "Core/PreRequisites.h"
#include "Layer.hpp"
#include "Utils/TimeGUI.hpp"
#include <memory>

struct TEVector2;

namespace TE
{

// Declare a dynamic delegate
DECLARE_MULTICAST_DELEGATE(OnLogoFinished)

class TE_API LogoLayer : public Layer
{
public:
    LogoLayer(const std::string &name = "Logo Layer");
    virtual ~LogoLayer();

    virtual void OnTimeGUIRender() override;
    virtual void OnDetach() override;

    OnLogoFinished LogoFinishedDelegate;

private:
    void OnWelcomeAnimationComplete();
    // LEGACY ANIMATION PROCEDURAL TESTING - COMMENTED OUT
    // void DrawTimeEngineLogo(const TEVector2 &center, float radius, TimeGUI::TimeGUIDrawList drawList, float time);

private:
    float m_Time = 0.0f;
    bool m_AnimationStarted = false;
    bool m_AnimationFinished = false;

    // LEGACY ANIMATION PROCEDURAL TESTING - COMMENTED OUT UNUSED MEMBERS
    /*
    std::string m_DisplayText;
    size_t m_CharIndex = 0;
    */
    const std::string m_FullText = "Welcome to TimeEngine!"; // Retained for duration timing calculation

    float m_AnimationStartTime = 0.0f;
    float m_LetterInterval = 0.1f;
    float m_TextClearDelay = 1.0f;
    bool m_ShouldClose = false;
    bool m_ShouldBroadcast = false;
    bool m_IsBeingRemoved = false;
};
} // namespace TE
