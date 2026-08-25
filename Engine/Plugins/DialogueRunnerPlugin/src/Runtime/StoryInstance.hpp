#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Core/Core.h"
#include "Graph/DialogueGraph.hpp"
#include "Interpreters/InkBytecodeRunner.hpp"
#include "Interpreters/YarnNodeRunner.hpp"
#include "NarrativeTypes.hpp"
#include "Runtime/Localization/NarrativeStringTable.hpp"
#include "Runtime/NarrativeBlackboard.hpp"
#include "Runtime/NarrativeValue.hpp"
#include "Runtime/QuestManager.hpp"
#include <functional>



using NarrativeExternalFunction = std::function<NarrativeValue(const TEArray<NarrativeValue> &args)>;

enum class StorySourceType : uint8_t
{
    Graph = 0,
    InkJson,
    Yarn
};

class StoryInstance
{
public:
    StoryInstance();
    ~StoryInstance() = default;

    bool LoadFromGraph(const DialogueGraph &graph);
    bool LoadFromInkJson(const TEString &inkJson);
    bool LoadFromYarn(const TEString &yarnText);
    void Reset();

    bool CanContinue() const;
    TEString Continue();

    const TEString &GetCurrentSpeaker() const;
    TEArray<StoryChoice> GetCurrentChoices() const;
    bool ChooseChoiceIndex(int index);

    bool JumpToKnot(const TEString &knotName);
    uint64_t GetCurrentGraphNodeID() const { return m_CurrentNodeID; }

    void BindFunction(const TEString &name, NarrativeExternalFunction func);
    NarrativeValue CallFunction(const TEString &name, const TEArray<NarrativeValue> &args);

    NarrativeBlackboard &GetBlackboard() { return m_Blackboard; }
    const NarrativeBlackboard &GetBlackboard() const { return m_Blackboard; }

    QuestManager &GetQuestManager() { return m_QuestManager; }
    const QuestManager &GetQuestManager() const { return m_QuestManager; }

    NarrativeStringTable &GetStringTable() { return m_StringTable; }
    const NarrativeStringTable &GetStringTable() const { return m_StringTable; }

    TEString SaveStateNativeText() const;
    bool LoadStateNativeText(const TEString &text);

    // Compatibility aliases
    TEString SaveStateJson() const { return SaveStateNativeText(); }
    bool LoadStateJson(const TEString &text) { return LoadStateNativeText(text); }

private:
    void StepGraph();

    StorySourceType m_SourceType = StorySourceType::Graph;
    DialogueGraph m_Graph;
    uint64_t m_CurrentNodeID = 0;
    TEString m_CurrentSpeaker;
    TEString m_CurrentText;
    TEArray<StoryChoice> m_CurrentChoices;
    bool m_bCanContinue = false;

    InkBytecodeRunner m_InkRunner;
    YarnNodeRunner m_YarnRunner;

    NarrativeBlackboard m_Blackboard;
    QuestManager m_QuestManager;
    NarrativeStringTable m_StringTable;

    TEMap<TEString, NarrativeExternalFunction> m_ExternalFunctions;
};

