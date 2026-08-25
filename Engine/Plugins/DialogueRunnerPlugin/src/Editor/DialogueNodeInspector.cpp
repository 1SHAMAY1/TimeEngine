#include "DialogueNodeInspector.hpp"
#include "Utils/TimeGUI.hpp"


void DialogueNodeInspector::DrawInspector(DialogueGraph &graph, uint64_t selectedNodeId)
{
    auto *node = graph.FindNode(selectedNodeId);
    if (!node)
    {
        TimeGUI::TextDisabled("Select a node on the graph canvas to inspect properties.");
        return;
    }

    TimeGUI::Text("Node Properties (ID: %llu)", node->ID);
    TimeGUI::Separator();

    // Title
    TimeGUI::InputText("Title", node->Title);

    // Node Type description
    const char *typeStr = "Unknown";
    switch (node->Type)
    {
    case NarrativeNodeType::Entry:
        typeStr = "Entry / Start";
        break;
    case NarrativeNodeType::Dialogue:
        typeStr = "Dialogue Line";
        break;
    case NarrativeNodeType::Choice:
        typeStr = "Branching Choices";
        break;
    case NarrativeNodeType::Condition:
        typeStr = "Condition Check";
        break;
    case NarrativeNodeType::Action:
        typeStr = "Action / Mutation / Quest";
        break;
    case NarrativeNodeType::Divert:
        typeStr = "Divert / Jump";
        break;
    case NarrativeNodeType::Exit:
        typeStr = "Exit / End";
        break;
    }
    TimeGUI::Text("Type: %s", typeStr);
    TimeGUI::Separator();

    if (node->Type == NarrativeNodeType::Dialogue)
    {
        TimeGUI::InputText("Speaker", node->Speaker);
        TimeGUI::InputText("Loc Key", node->LocalizedKey);

        TimeGUI::Text("Dialogue Text:");
        TimeGUI::InputTextMultiline("##DiagText", node->Text, TEVector2(0, 100));

        TimeGUI::InputText("Portrait Tag", node->PortraitTag);
    }
    else if (node->Type == NarrativeNodeType::Choice)
    {
        TimeGUI::InputText("Speaker", node->Speaker);

        TimeGUI::Separator();
        TimeGUI::Text("Choice Options:");

        for (size_t c = 0; c < node->Choices.Size(); ++c)
        {
            TimeGUI::PushID(static_cast<int>(c));
            TEString label = "Choice " + TEString::FromInt(static_cast<int>(c) + 1);
            if (TimeGUI::InputText(label, node->Choices[c].Text))
            {
                if (c < node->OutputPins.Size())
                {
                    node->OutputPins[c].Name = node->Choices[c].Text;
                }
            }

            TimeGUI::SameLine();
            if (TimeGUI::Button("X") && node->Choices.Size() > 1)
            {
                if (c < node->OutputPins.Size())
                {
                    graph.DisconnectPin(node->OutputPins[c].ID);
                    node->OutputPins.RemoveAt(c);
                }
                node->Choices.RemoveAt(c);
                TimeGUI::PopID();
                break;
            }
            TimeGUI::PopID();
        }

        if (TimeGUI::Button("+ Add Choice Option"))
        {
            StoryChoice newChoice;
            newChoice.Index = static_cast<int>(node->Choices.Size());
            newChoice.Text = "New Option";
            node->Choices.Add(newChoice);

            DialogueGraphPin pin(graph.GenerateID(), node->ID, "Option " + TEString::FromInt(newChoice.Index + 1),
                                 PinType::ChoiceOption, PinDirection::Output);
            node->OutputPins.Add(pin);
        }
    }
    else if (node->Type == NarrativeNodeType::Condition)
    {
        TimeGUI::InputText("Variable", node->ConditionVar);

        int opIdx = static_cast<int>(node->ConditionOp);
        const char *ops[] = {"== (Equal)", "!= (Not Equal)", "> (Greater)", "< (Less)", ">= (Greater Equal)", "<= (Less Equal)"};
        if (TimeGUI::Combo("Comparison", &opIdx, ops, 6))
        {
            node->ConditionOp = static_cast<ComparisonOp>(opIdx);
        }

        TEString valStr = node->ConditionValue.AsString();
        if (TimeGUI::InputText("Target Value", valStr))
        {
            node->ConditionValue = NarrativeValue::Parse(valStr);
        }
    }
    else if (node->Type == NarrativeNodeType::Action)
    {
        TimeGUI::InputText("Variable", node->ActionVar);

        int opIdx = static_cast<int>(node->ActionOp);
        const char *ops[] = {"Set (=)", "Add (+)", "Subtract (-)", "Multiply (*)", "Toggle (NOT)"};
        if (TimeGUI::Combo("Operation", &opIdx, ops, 5))
        {
            node->ActionOp = static_cast<MutationOp>(opIdx);
        }

        TEString valStr = node->ActionValue.AsString();
        if (TimeGUI::InputText("Value", valStr))
        {
            node->ActionValue = NarrativeValue::Parse(valStr);
        }

        TimeGUI::Separator();
        TimeGUI::Text("Custom Event Hook:");
        TimeGUI::InputText("Function", node->CustomFunction);
        TimeGUI::InputText("Arguments", node->CustomArgs);

        TimeGUI::Separator();
        TimeGUI::Text("Quest Modification:");
        TimeGUI::InputText("Quest ID", node->QuestID);

        int qStatus = static_cast<int>(node->TargetQuestStatus);
        const char *qStatuses[] = {"Not Started", "Active", "Completed", "Failed"};
        if (TimeGUI::Combo("Quest Status", &qStatus, qStatuses, 4))
        {
            node->TargetQuestStatus = static_cast<QuestStatus>(qStatus);
        }

        TimeGUI::InputText("Objective ID", node->ObjectiveID);
    }
    else if (node->Type == NarrativeNodeType::Divert)
    {
        TimeGUI::InputText("Target Knot", node->DivertTargetKnot);
    }
}

