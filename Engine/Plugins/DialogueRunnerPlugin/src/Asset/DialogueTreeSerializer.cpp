#include "DialogueTreeSerializer.hpp"
#include "Core/Log.h"



static TEString EscapePipes(const TEString &str)
{
    TEString result;
    result.reserve(str.size() + 8);
    for (char c : str)
    {
        if (c == '|')
            result += "\\|";
        else if (c == '\\')
            result += "\\\\";
        else if (c == '\n')
            result += "\\n";
        else if (c == '\r')
            result += "\\r";
        else
            result += c;
    }
    return result;
}

static TEString UnescapePipes(const TEString &str)
{
    TEString result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\\' && i + 1 < str.size())
        {
            char next = str[i + 1];
            if (next == '|')
                result += '|';
            else if (next == '\\')
                result += '\\';
            else if (next == 'n')
                result += '\n';
            else if (next == 'r')
                result += '\r';
            else
                result += next;
            ++i;
        }
        else
        {
            result += str[i];
        }
    }
    return result;
}

static TEArray<TEString> SplitPipe(const TEString &line)
{
    TEArray<TEString> parts;
    TEString cur;
    for (size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '\\' && i + 1 < line.size() && (line[i + 1] == '|' || line[i + 1] == '\\'))
        {
            cur += line[i];
            cur += line[i + 1];
            ++i;
        }
        else if (line[i] == '|')
        {
            parts.Add(UnescapePipes(cur));
            cur.Clear();
        }
        else
        {
            cur += line[i];
        }
    }
    parts.Add(UnescapePipes(cur));
    return parts;
}

TEString DialogueTreeSerializer::SerializeToNativeText(const DialogueGraph &graph)
{
    TEString result;
    result += "DialogueTree: Version 1\n";
    result += "EntryNodeID: " + TEString(std::to_string(graph.GetEntryNodeID()).c_str()) + "\n";

    const auto &nodes = graph.GetNodes();
    result += "NodeCount: " + TEString::FromInt((int)nodes.Size()) + "\n";
    for (size_t i = 0; i < nodes.Size(); ++i)
    {
        const auto &n = nodes[i];
        result += TEString("Node: ")
            + TEString(std::to_string(n.ID).c_str()) + "|"
            + EscapePipes(n.Title) + "|"
            + TEString::FromInt(static_cast<int>(n.Type)) + "|"
            + TEString::FromFloat(n.Position.x) + " " + TEString::FromFloat(n.Position.y) + "|"
            + EscapePipes(n.Speaker) + "|"
            + EscapePipes(n.Text) + "|"
            + EscapePipes(n.LocalizedKey) + "|"
            + EscapePipes(n.PortraitTag) + "|"
            + EscapePipes(n.ConditionVar) + "|"
            + TEString::FromInt(static_cast<int>(n.ConditionOp)) + "|"
            + EscapePipes(n.ConditionValue.AsString()) + "|"
            + EscapePipes(n.ActionVar) + "|"
            + TEString::FromInt(static_cast<int>(n.ActionOp)) + "|"
            + EscapePipes(n.ActionValue.AsString()) + "|"
            + EscapePipes(n.CustomFunction) + "|"
            + EscapePipes(n.CustomArgs) + "|"
            + EscapePipes(n.QuestID) + "|"
            + TEString::FromInt(static_cast<int>(n.TargetQuestStatus)) + "|"
            + EscapePipes(n.ObjectiveID) + "|"
            + EscapePipes(n.DivertTargetKnot) + "\n";

        for (size_t c = 0; c < n.Choices.Size(); ++c)
        {
            const auto &ch = n.Choices[c];
            result += TEString("Choice: ") + TEString(std::to_string(n.ID).c_str()) + "|" + TEString::FromInt(ch.Index) + "|" + EscapePipes(ch.Text) + "|" + EscapePipes(ch.ConditionExpression) + "\n";
        }

        for (size_t p = 0; p < n.InputPins.Size(); ++p)
        {
            const auto &pin = n.InputPins[p];
            result += TEString("Pin: ") + TEString(std::to_string(n.ID).c_str()) + "|" + TEString(std::to_string(pin.ID).c_str()) + "|" + EscapePipes(pin.Name) + "|" + TEString(std::to_string(static_cast<int>(pin.Type.GetKind())).c_str()) + "|0\n";
        }

        for (size_t p = 0; p < n.OutputPins.Size(); ++p)
        {
            const auto &pin = n.OutputPins[p];
            result += TEString("Pin: ") + TEString(std::to_string(n.ID).c_str()) + "|" + TEString(std::to_string(pin.ID).c_str()) + "|" + EscapePipes(pin.Name) + "|" + TEString(std::to_string(static_cast<int>(pin.Type.GetKind())).c_str()) + "|1\n";
        }
    }

    const auto &conns = graph.GetConnections();
    result += "ConnectionCount: " + TEString::FromInt((int)conns.Size()) + "\n";
    for (size_t i = 0; i < conns.Size(); ++i)
    {
        const auto &c = conns[i];
        result += TEString("Connection: ") + TEString(std::to_string(c.ID).c_str()) + "|" + TEString(std::to_string(c.SourceNodeID).c_str()) + "|" + TEString(std::to_string(c.SourcePinID).c_str()) + "|" + TEString(std::to_string(c.TargetNodeID).c_str()) + "|" + TEString(std::to_string(c.TargetPinID).c_str()) + "\n";
    }

    return result;
}

bool DialogueTreeSerializer::DeserializeFromNativeText(DialogueGraph &graph, const TEString &text)
{
    if (text.empty())
        return false;

    graph.Clear();

    TEArray<TEString> lines = text.Split('\n');
    for (TEString line : lines)
    {
        line = line.Trim();
        if (line.empty() || line.find("//") == 0)
            continue;

        if (line.find("EntryNodeID: ") == 0)
        {
            uint64_t entryId = std::stoull(line.substr(13));
            graph.SetEntryNodeID(entryId);
        }
        else if (line.find("Node: ") == 0)
        {
            TEString data = line.substr(6);
            auto parts = SplitPipe(data);
            if (parts.Num() >= 20)
            {
                DialogueGraphNode node;
                node.ID = std::stoull(parts[0]);
                node.Title = parts[1];
                node.Type = static_cast<NarrativeNodeType>(std::stoi(parts[2]));

                auto posParts = parts[3].Split(' ');
                if (posParts.Num() >= 2)
                {
                    node.Position.x = posParts[0].ToFloat();
                    node.Position.y = posParts[1].ToFloat();
                }

                node.Speaker = parts[4];
                node.Text = parts[5];
                node.LocalizedKey = parts[6];
                node.PortraitTag = parts[7];
                node.ConditionVar = parts[8];
                node.ConditionOp = static_cast<ComparisonOp>(std::stoi(parts[9]));
                node.ConditionValue = NarrativeValue::Parse(parts[10]);
                node.ActionVar = parts[11];
                node.ActionOp = static_cast<MutationOp>(std::stoi(parts[12]));
                node.ActionValue = NarrativeValue::Parse(parts[13]);
                node.CustomFunction = parts[14];
                node.CustomArgs = parts[15];
                node.QuestID = parts[16];
                node.TargetQuestStatus = static_cast<QuestStatus>(std::stoi(parts[17]));
                node.ObjectiveID = parts[18];
                node.DivertTargetKnot = parts[19];

                graph.GetNodes().Add(node);
            }
        }
        else if (line.find("Choice: ") == 0)
        {
            TEString data = line.substr(8);
            auto parts = SplitPipe(data);
            if (parts.Num() >= 4)
            {
                uint64_t nodeId = std::stoull(parts[0]);
                auto *node = graph.FindNode(nodeId);
                if (node)
                {
                    StoryChoice ch;
                    ch.Index = std::stoi(parts[1]);
                    ch.Text = parts[2];
                    ch.ConditionExpression = parts[3];
                    node->Choices.Add(ch);
                }
            }
        }
        else if (line.find("Pin: ") == 0)
        {
            TEString data = line.substr(5);
            auto parts = SplitPipe(data);
            if (parts.Num() >= 5)
            {
                uint64_t nodeId = std::stoull(parts[0]);
                auto *node = graph.FindNode(nodeId);
                if (node)
                {
                    DialogueGraphPin pin;
                    pin.NodeID = nodeId;
                    pin.ID = std::stoull(parts[1]);
                    pin.Name = parts[2];
                    pin.Type.SetKind(static_cast<GraphPinKind>(std::stoi(parts[3])));
                    pin.Direction = (std::stoi(parts[4]) == 0) ? PinDirection::Input : PinDirection::Output;

                    if (pin.Direction == PinDirection::Input)
                        node->InputPins.Add(pin);
                    else
                        node->OutputPins.Add(pin);
                }
            }
        }
        else if (line.find("Connection: ") == 0)
        {
            TEString data = line.substr(12);
            auto parts = SplitPipe(data);
            if (parts.Num() >= 5)
            {
                DialogueGraphConnection conn;
                conn.ID = std::stoull(parts[0]);
                conn.SourceNodeID = std::stoull(parts[1]);
                conn.SourcePinID = std::stoull(parts[2]);
                conn.TargetNodeID = std::stoull(parts[3]);
                conn.TargetPinID = std::stoull(parts[4]);

                if (conn.SourceNodeID != 0 && conn.TargetNodeID != 0)
                {
                    graph.GetConnections().Add(conn);
                }
            }
        }
    }

    return true;
}

bool DialogueTreeSerializer::SaveToFile(const DialogueGraph &graph, const TEString &filepath)
{
    TEString content = SerializeToNativeText(graph);
    std::ofstream out(filepath.c_str());
    if (!out.is_open())
    {
        TE_CORE_ERROR("DialogueTreeSerializer: Failed to write file at {0}", filepath.c_str());
        return false;
    }
    out << content.c_str();
    return true;
}

bool DialogueTreeSerializer::LoadFromFile(DialogueGraph &graph, const TEString &filepath)
{
    std::ifstream in(filepath.c_str());
    if (!in.is_open())
        return false;

    in.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(in.tellg());
    in.seekg(0, std::ios::beg);

    TEArray<char> buffer;
    buffer.Resize(size + 1, '\0');
    in.read(buffer.Data(), size);
    TEString content = buffer.Data();

    return DeserializeFromNativeText(graph, content);
}

