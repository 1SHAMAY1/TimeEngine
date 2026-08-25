#include "QuestManager.hpp"
#include <sstream>


void QuestManager::RegisterQuest(const QuestData &quest)
{
    m_Quests[quest.ID] = quest;
}

void QuestManager::UnregisterQuest(const TEString &questID)
{
    m_Quests.Remove(questID);
}

bool QuestManager::HasQuest(const TEString &questID) const
{
    return m_Quests.Find(questID) != nullptr;
}

QuestData *QuestManager::GetQuest(const TEString &questID)
{
    return m_Quests.Find(questID);
}

const QuestData *QuestManager::GetQuest(const TEString &questID) const
{
    return m_Quests.Find(questID);
}

void QuestManager::SetQuestStatus(const TEString &questID, QuestStatus status)
{
    auto *quest = GetQuest(questID);
    if (quest)
    {
        if (quest->Status != status)
        {
            quest->Status = status;
            for (auto &cb : m_Observers)
            {
                if (cb)
                    cb(questID, status);
            }
        }
    }
}

QuestStatus QuestManager::GetQuestStatus(const TEString &questID) const
{
    const auto *quest = GetQuest(questID);
    if (quest)
        return quest->Status;
    return QuestStatus::NotStarted;
}

bool QuestManager::SetObjectiveProgress(const TEString &questID, const TEString &objectiveID, int progress)
{
    auto *quest = GetQuest(questID);
    if (!quest)
        return false;

    for (size_t i = 0; i < quest->Objectives.Size(); ++i)
    {
        if (quest->Objectives[i].ID == objectiveID)
        {
            quest->Objectives[i].CurrentProgress = progress;
            if (quest->Objectives[i].CurrentProgress >= quest->Objectives[i].TargetProgress)
            {
                quest->Objectives[i].bIsCompleted = true;
            }
            return true;
        }
    }
    return false;
}

bool QuestManager::CompleteObjective(const TEString &questID, const TEString &objectiveID)
{
    auto *quest = GetQuest(questID);
    if (!quest)
        return false;

    for (size_t i = 0; i < quest->Objectives.Size(); ++i)
    {
        if (quest->Objectives[i].ID == objectiveID)
        {
            quest->Objectives[i].bIsCompleted = true;
            quest->Objectives[i].CurrentProgress = quest->Objectives[i].TargetProgress;
            return true;
        }
    }
    return false;
}

void QuestManager::AddObserver(QuestStatusChangeCallback callback)
{
    m_Observers.push_back(callback);
}

void QuestManager::ClearObservers()
{
    m_Observers.clear();
}

void QuestManager::Clear()
{
    m_Quests.Clear();
}

TEString QuestManager::SerializeNativeText() const
{
    std::ostringstream ss;
    ss << "QuestCount: " << m_Quests.Size() << "\n";
    for (auto it = m_Quests.begin(); it != m_Quests.end(); ++it)
    {
        const auto &q = it->second;
        ss << "Quest: " << q.ID << "|" << q.Title << "|" << q.Description << "|" << static_cast<int>(q.Status) << "\n";
        for (size_t i = 0; i < q.Objectives.Size(); ++i)
        {
            const auto &obj = q.Objectives[i];
            ss << "QuestObj: " << q.ID << "|" << obj.ID << "|" << obj.Description << "|"
               << (obj.bIsCompleted ? 1 : 0) << "|" << obj.CurrentProgress << "|" << obj.TargetProgress << "\n";
        }
    }
    return ss.str();
}

bool QuestManager::DeserializeNativeText(const TEString &text)
{
    Clear();
    if (text.empty())
        return true;

    TEArray<TEString> lines = text.Split('\n');
    for (TEString line : lines)
    {
        line = line.Trim();

        if (line.find("Quest: ") == 0)
        {
            TEString content = line.substr(7);
            TEArray<TEString> parts = content.Split('|');
            if (parts.Num() >= 4)
            {
                QuestData q;
                q.ID = parts[0];
                q.Title = parts[1];
                q.Description = parts[2];
                q.Status = static_cast<QuestStatus>(std::stoi(parts[3]));
                m_Quests[parts[0]] = q;
            }
        }
        else if (line.find("QuestObj: ") == 0)
        {
            TEString content = line.substr(10);
            TEArray<TEString> parts = content.Split('|');
            if (parts.Num() >= 6)
            {
                auto *q = GetQuest(parts[0]);
                if (q)
                {
                    QuestObjective obj;
                    obj.ID = parts[1];
                    obj.Description = parts[2];
                    obj.bIsCompleted = (std::stoi(parts[3]) != 0);
                    obj.CurrentProgress = std::stoi(parts[4]);
                    obj.TargetProgress = std::stoi(parts[5]);
                    q->Objectives.Add(obj);
                }
            }
        }
    }
    return true;
}

