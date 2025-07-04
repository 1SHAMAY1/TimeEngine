#pragma once

#include "Core/PreRequisites.h"
#include "Layer.hpp"

struct ImDrawList;
struct ImVec2;

namespace TE {

    // Declare a dynamic delegate
    DECLARE_MULTICAST_DELEGATE(OnLogoFinished)

    class TE_API LogoLayer : public Layer {
    public:
        LogoLayer(const std::string& name = "Logo Layer");
        virtual ~LogoLayer();

        virtual void OnImGuiRender() override;
        virtual void OnDetach() override;

        OnLogoFinished LogoFinishedDelegate;

    private:
        void OnWelcomeAnimationComplete();
        void DrawTimeEngineLogo(const ImVec2& center, float radius, ImDrawList* drawList, float time);

    private:
        float m_Time = 0.0f;
        bool m_AnimationStarted = false;
        bool m_AnimationFinished = false;
        std::string m_DisplayText;
        const std::string m_FullText = "Welcome to TimeEngine!";
        size_t m_CharIndex = 0;
        float m_AnimationStartTime = 0.0f;
        float m_LetterInterval = 0.1f;
        float m_TextClearDelay = 1.0f;
        bool m_ShouldClose = false;
    };
}
