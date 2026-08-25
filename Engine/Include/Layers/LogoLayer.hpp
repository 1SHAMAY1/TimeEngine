#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Layer.hpp"
#include "Utils/TimeGUI.hpp"

struct TEVector2;


// Declare a dynamic delegate
DECLARE_MULTICAST_DELEGATE(OnLogoFinished)

class TE_API LogoLayer : public Layer
{
public:
    LogoLayer(const TEString &name = "Logo Layer");
    virtual ~LogoLayer();

    virtual void OnTimeGUIRender() override;
    virtual void OnDetach() override;

    OnLogoFinished LogoFinishedDelegate;

private:
    void OnWelcomeAnimationComplete();

private:
    float m_Time = 0.0f;
    float m_DisplayProgress = 0.0f;
    float m_TargetProgress = 0.0f;
    TEString m_StatusText = "Initializing TimeEngine Systems...";
    bool m_PluginsLoaded = false;
    float m_FinishTime = 0.0f;

    bool m_AnimationStarted = false;
    bool m_AnimationFinished = false;
    bool m_ShouldClose = false;
    bool m_ShouldBroadcast = false;
    bool m_IsBeingRemoved = false;
};
