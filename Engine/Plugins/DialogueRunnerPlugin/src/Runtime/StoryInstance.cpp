#include "StoryInstance.hpp"
#include <sstream>


StoryInstance::StoryInstance()
{
    Reset();
}

bool StoryInstance::LoadFromGraph(const DialogueGraph &graph)
{
    Reset();
    m_SourceType = StorySourceType::Graph;
    m_Graph = graph;
    m_CurrentNodeID = m_Graph.GetEntryNodeID();

    if (m_CurrentNodeID == 0 && m_Graph.GetNodes().Size() > 0)
    {
        m_CurrentNodeID = m_Graph.GetNodes()[0].ID;
    }

    m_bCanContinue = (m_CurrentNodeID != 0);
    return m_bCanContinue;
}

bool StoryInstance::LoadFromInkJson(const TEString &inkJson)
{
    Reset();
    m_SourceType = StorySourceType::InkJson;
    m_bCanContinue = m_InkRunner.LoadFromJson(inkJson);
    return m_bCanContinue;
}

bool StoryInstance::LoadFromYarn(const TEString &yarnText)
{
    Reset();
    m_SourceType = StorySourceType::Yarn;
    m_bCanContinue = m_YarnRunner.LoadFromYarnSource(yarnText);
    return m_bCanContinue;
}

bool StoryInstance::CanContinue() const
{
    switch (m_SourceType)
    {
    case StorySourceType::Graph:
        return m_bCanContinue && m_CurrentNodeID != 0;
    case StorySourceType::InkJson:
        return m_InkRunner.CanContinue();
    case StorySourceType::Yarn:
        return m_YarnRunner.CanContinue();
    default:
        return false;
    }
}

TEString StoryInstance::Continue()
{
    if (m_SourceType == StorySourceType::InkJson)
    {
        TEString raw = m_InkRunner.Continue(&m_Blackboard);
        m_CurrentSpeaker = m_InkRunner.GetCurrentSpeaker();
        return m_StringTable.FormatString(raw, &m_Blackboard);
    }

    if (m_SourceType == StorySourceType::Yarn)
    {
        TEString raw = m_YarnRunner.Continue(&m_Blackboard);
        m_CurrentSpeaker = m_YarnRunner.GetCurrentSpeaker();
        return m_StringTable.FormatString(raw, &m_Blackboard);
    }

    // Graph evaluation
    m_CurrentChoices.Clear();
    m_CurrentText.Clear();

    while (m_CurrentNodeID != 0)
    {
        auto *node = m_Graph.FindNode(m_CurrentNodeID);
        if (!node)
        {
            m_CurrentNodeID = 0;
            m_bCanContinue = false;
            break;
        }

        switch (node->Type)
        {
        case NarrativeNodeType::Entry: {
            // Advance to next connected node
            m_CurrentNodeID = m_Graph.GetNextNode(node->ID);
            continue;
        }

        case NarrativeNodeType::Dialogue: {
            m_CurrentSpeaker = node->Speaker;
            TEString raw = node->Text;
            if (!node->LocalizedKey.empty() && m_StringTable.HasString(node->LocalizedKey))
            {
                raw = m_StringTable.GetRawString(node->LocalizedKey);
            }
            m_CurrentText = m_StringTable.FormatString(raw, &m_Blackboard);

            // Advance pointer to next node for future continue
            m_CurrentNodeID = m_Graph.GetNextNode(node->ID);
            m_bCanContinue = (m_CurrentNodeID != 0);
            return m_CurrentText;
        }

        case NarrativeNodeType::Choice: {
            m_CurrentSpeaker = node->Speaker;
            // Gather choices with their target connection node IDs
            auto conns = m_Graph.GetConnectionsFromNode(node->ID);
            for (size_t c = 0; c < node->Choices.Size(); ++c)
            {
                StoryChoice choice = node->Choices[c];
                choice.Index = static_cast<int>(c);

                // Find connection attached to output pin for this choice
                if (c < node->OutputPins.Size())
                {
                    uint64_t pinId = node->OutputPins[c].ID;
                    for (size_t k = 0; k < conns.Size(); ++k)
                    {
                        if (conns[k].SourcePinID == pinId)
                        {
                            choice.TargetNodeID = conns[k].TargetNodeID;
                            break;
                        }
                    }
                }
                m_CurrentChoices.push_back(choice);
            }

            m_bCanContinue = false; // Pause at choice
            return "";
        }

        case NarrativeNodeType::Condition: {
            bool eval = m_Blackboard.EvaluateCondition(node->ConditionVar, node->ConditionOp, node->ConditionValue);
            uint64_t truePinId = node->OutputPins.Size() > 0 ? node->OutputPins[0].ID : 0;
            uint64_t falsePinId = node->OutputPins.Size() > 1 ? node->OutputPins[1].ID : 0;

            uint64_t nextPin = eval ? truePinId : falsePinId;
            m_CurrentNodeID = m_Graph.GetNextNode(node->ID, nextPin);
            continue;
        }

        case NarrativeNodeType::Action: {
            if (!node->ActionVar.empty())
            {
                m_Blackboard.ApplyMutation(node->ActionVar, node->ActionOp, node->ActionValue);
            }

            if (!node->CustomFunction.empty())
            {
                TEArray<NarrativeValue> args;
                if (!node->CustomArgs.empty())
                    args.Add(NarrativeValue::Parse(node->CustomArgs));
                CallFunction(node->CustomFunction, args);
            }

            if (!node->QuestID.empty())
            {
                m_QuestManager.SetQuestStatus(node->QuestID, node->TargetQuestStatus);
                if (!node->ObjectiveID.empty())
                {
                    m_QuestManager.CompleteObjective(node->QuestID, node->ObjectiveID);
                }
            }

            m_CurrentNodeID = m_Graph.GetNextNode(node->ID);
            continue;
        }

        case NarrativeNodeType::Divert: {
            if (!node->DivertTargetKnot.empty())
            {
                JumpToKnot(node->DivertTargetKnot);
            }
            else
            {
                m_CurrentNodeID = m_Graph.GetNextNode(node->ID);
            }
            continue;
        }

        case NarrativeNodeType::Exit: {
            m_CurrentNodeID = 0;
            m_bCanContinue = false;
            return "";
        }
        }
    }

    m_bCanContinue = false;
    return "";
}

const TEString &StoryInstance::GetCurrentSpeaker() const
{
    return m_CurrentSpeaker;
}

TEArray<StoryChoice> StoryInstance::GetCurrentChoices() const
{
    if (m_SourceType == StorySourceType::InkJson)
    {
        TEArray<StoryChoice> res;
        const auto &inkChoices = m_InkRunner.GetCurrentChoices();
        for (const auto &ic : inkChoices)
        {
            StoryChoice c;
            c.Index = ic.Index;
            c.Text = ic.Text;
            c.TargetKnot = ic.TargetPath;
            res.Add(c);
        }
        return res;
    }

    if (m_SourceType == StorySourceType::Yarn)
    {
        return m_YarnRunner.GetCurrentChoices();
    }

    return m_CurrentChoices;
}

bool StoryInstance::ChooseChoiceIndex(int index)
{
    if (m_SourceType == StorySourceType::InkJson)
    {
        return m_InkRunner.ChooseChoiceIndex(index);
    }

    if (m_SourceType == StorySourceType::Yarn)
    {
        return m_YarnRunner.ChooseChoiceIndex(index);
    }

    if (index < 0 || index >= static_cast<int>(m_CurrentChoices.size()))
        return false;

    uint64_t targetNodeId = m_CurrentChoices[index].TargetNodeID;
    m_CurrentChoices.clear();

    if (targetNodeId != 0)
    {
        m_CurrentNodeID = targetNodeId;
        m_bCanContinue = true;
        return true;
    }

    m_bCanContinue = false;
    return false;
}

bool StoryInstance::JumpToKnot(const TEString &knotName)
{
    if (m_SourceType == StorySourceType::InkJson)
    {
        return m_InkRunner.JumpToPath(knotName);
    }
    if (m_SourceType == StorySourceType::Yarn)
    {
        return m_YarnRunner.JumpToNode(knotName);
    }

    // Search graph nodes by Title or DivertTargetKnot
    const auto &nodes = m_Graph.GetNodes();
    for (size_t i = 0; i < nodes.Size(); ++i)
    {
        if (nodes[i].Title == knotName || nodes[i].DivertTargetKnot == knotName)
        {
            m_CurrentNodeID = nodes[i].ID;
            m_bCanContinue = true;
            return true;
        }
    }
    return false;
}

void StoryInstance::BindFunction(const TEString &name, NarrativeExternalFunction func)
{
    m_ExternalFunctions[name] = func;
}

NarrativeValue StoryInstance::CallFunction(const TEString &name, const TEArray<NarrativeValue> &args)
{
    auto *found = m_ExternalFunctions.Find(name);
    if (found && *found)
    {
        return (*found)(args);
    }
    return NarrativeValue();
}

void StoryInstance::Reset()
{
    m_SourceType = StorySourceType::Graph;
    m_CurrentNodeID = 0;
    m_CurrentSpeaker.Clear();
    m_CurrentText.Clear();
    m_CurrentChoices.Clear();
    m_bCanContinue = false;
    m_InkRunner.Reset();
    m_YarnRunner.Reset();
}

TEString StoryInstance::SaveStateNativeText() const
{
    TEString result;
    result += "StoryState: Version 1\n";
    result += "CurrentNodeID: " + TEString(std::to_string(m_CurrentNodeID).c_str()) + "\n";
    result += m_Blackboard.SerializeNativeText();
    result += m_QuestManager.SerializeNativeText();
    return result;
}

bool StoryInstance::LoadStateNativeText(const TEString &text)
{
    if (text.empty())
        return false;

    TEArray<TEString> lines = text.Split('\n');
    for (TEString line : lines)
    {
        line = line.Trim();

        if (line.find("CurrentNodeID: ") == 0)
        {
            m_CurrentNodeID = std::stoull(line.substr(15));
            m_bCanContinue = (m_CurrentNodeID != 0);
        }
    }

    m_Blackboard.DeserializeNativeText(text);
    m_QuestManager.DeserializeNativeText(text);
    return true;
}

