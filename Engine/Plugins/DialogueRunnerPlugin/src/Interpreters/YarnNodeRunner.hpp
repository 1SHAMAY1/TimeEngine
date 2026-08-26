#pragma once

#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "NarrativeTypes.hpp"
#include "Runtime/NarrativeBlackboard.hpp"

struct YarnNode
{
    TEString Title;
    TEArray<TEString> Tags;
    TEArray<TEString> BodyLines;
};

class YarnNodeRunner
{
public:
    YarnNodeRunner() = default;
    ~YarnNodeRunner() = default;

    bool LoadFromYarnSource(const TEString &yarnText);
    bool CanContinue() const;
    TEString Continue(NarrativeBlackboard *blackboard = nullptr);

    const TEArray<StoryChoice> &GetCurrentChoices() const { return m_CurrentChoices; }
    bool ChooseChoiceIndex(int index);

    bool JumpToNode(const TEString &nodeTitle);
    void Reset();

    const TEString &GetCurrentSpeaker() const { return m_CurrentSpeaker; }
    const TEString &GetCurrentNodeTitle() const { return m_CurrentNodeTitle; }

private:
    void ParseNodes(const TEString &yarnText);
    void EvaluateCommandLine(const TEString &cmd, NarrativeBlackboard *blackboard);

    TEMap<TEString, YarnNode> m_Nodes;
    TEString m_CurrentNodeTitle;
    size_t m_CurrentLineIndex = 0;
    TEString m_CurrentSpeaker;
    TEArray<StoryChoice> m_CurrentChoices;
    bool m_bCanContinue = false;
};
