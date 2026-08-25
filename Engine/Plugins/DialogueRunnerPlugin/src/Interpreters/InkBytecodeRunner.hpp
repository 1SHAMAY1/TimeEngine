#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Core/Core.h"
#include "NarrativeTypes.hpp"
#include "Runtime/NarrativeBlackboard.hpp"
#include "Runtime/NarrativeValue.hpp"



struct InkChoicePoint
{
    int Index = 0;
    TEString Text;
    TEString TargetPath;
    bool bIsOnceOnly = false;
};

class InkBytecodeRunner
{
public:
    InkBytecodeRunner() = default;
    ~InkBytecodeRunner() = default;

    bool LoadFromJson(const TEString &inkJson);
    bool CanContinue() const;
    TEString Continue(NarrativeBlackboard *blackboard = nullptr);

    const TEArray<InkChoicePoint> &GetCurrentChoices() const { return m_CurrentChoices; }
    bool ChooseChoiceIndex(int index);

    bool JumpToPath(const TEString &path);
    void Reset();

    const TEString &GetCurrentSpeaker() const { return m_CurrentSpeaker; }
    const TEArray<TEString> &GetCurrentTags() const { return m_CurrentTags; }

private:
    void ExecuteNextInstruction(NarrativeBlackboard *blackboard);

    TEString m_RawJson;
    bool m_bCanContinue = false;
    size_t m_InstructionPointer = 0;
    TEString m_CurrentSpeaker;
    TEArray<TEString> m_CurrentTags;
    TEArray<InkChoicePoint> m_CurrentChoices;
    TEArray<TEString> m_Lines;
    TEMap<TEString, size_t> m_KnotAddresses;
    TEMap<TEString, int> m_VisitCounts;
};

