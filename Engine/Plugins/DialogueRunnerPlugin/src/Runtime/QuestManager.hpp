#pragma once

#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "NarrativeTypes.hpp"
#include <functional>

using QuestStatusChangeCallback = std::function<void(const TEString &questID, QuestStatus newStatus)>;

class QuestManager
{
public:
    QuestManager() = default;
    ~QuestManager() = default;

    void RegisterQuest(const QuestData &quest);
    void UnregisterQuest(const TEString &questID);
    bool HasQuest(const TEString &questID) const;

    QuestData *GetQuest(const TEString &questID);
    const QuestData *GetQuest(const TEString &questID) const;
    const TEMap<TEString, QuestData> &GetAllQuests() const { return m_Quests; }

    void SetQuestStatus(const TEString &questID, QuestStatus status);
    QuestStatus GetQuestStatus(const TEString &questID) const;

    bool SetObjectiveProgress(const TEString &questID, const TEString &objectiveID, int progress);
    bool CompleteObjective(const TEString &questID, const TEString &objectiveID);

    void AddObserver(QuestStatusChangeCallback callback);
    void ClearObservers();
    void Clear();

    TEString SerializeNativeText() const;
    bool DeserializeNativeText(const TEString &text);

    // Compatibility aliases
    TEString SerializeJson() const { return SerializeNativeText(); }
    bool DeserializeJson(const TEString &text) { return DeserializeNativeText(text); }

private:
    TEMap<TEString, QuestData> m_Quests;
    TEArray<QuestStatusChangeCallback> m_Observers;
};
