#pragma once

#include "Core/Graph/GraphNode.hpp"
#include "Graph/DialogueGraphPin.hpp"
#include "NarrativeTypes.hpp"
#include "Runtime/NarrativeValue.hpp"

struct DialogueGraphNode : public GraphNode
{
    NarrativeNodeType Type = NarrativeNodeType::Dialogue;

    // Dialogue Content
    TEString Speaker;
    TEString Text;
    TEString LocalizedKey;
    TEString PortraitTag;

    // Choice Options (when Type == Choice)
    TEArray<StoryChoice> Choices;

    // Condition Check (when Type == Condition)
    TEString ConditionVar;
    ComparisonOp ConditionOp = ComparisonOp::Equal;
    NarrativeValue ConditionValue;

    // Action / Mutation (when Type == Action)
    TEString ActionVar;
    MutationOp ActionOp = MutationOp::Set;
    NarrativeValue ActionValue;
    TEString CustomFunction;
    TEString CustomArgs;

    // Quest Hooks
    TEString QuestID;
    QuestStatus TargetQuestStatus = QuestStatus::Active;
    TEString ObjectiveID;

    // Divert / Knot Jump
    TEString DivertTargetKnot;

    DialogueGraphNode() { NodeType = "DialogueNode"; }
};
