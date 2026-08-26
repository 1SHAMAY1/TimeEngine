#include "Core/PreRequisites.h"
#include "Core/Graph/GraphSerializer.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

GraphSerializer::GraphSerializer(Graph &graph) : m_Graph(graph) {}

bool GraphSerializer::Serialize(const TEString &filepath)
{
    std::ofstream hout(filepath.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("GraphSerializer: Failed to open file for writing at {0}", filepath);
        return false;
    }

    TEString serialized;
    if (SerializeToString(serialized))
    {
        hout << serialized.c_str();
        hout.close();
        return true;
    }
    return false;
}

bool GraphSerializer::SerializeToString(TEString &outText)
{
    TEString ss;
    ss += "Graph: " + m_Graph.GetName() + "\n";
    ss += "NextID: " + TEString::FromInt64(static_cast<int64_t>(m_Graph.GetNextID())) + "\n";

    const auto &nodes = m_Graph.GetNodes();
    ss += "NodesCount: " + TEString::FromInt(static_cast<int>(nodes.Size())) + "\n";

    for (const auto &node : nodes)
    {
        ss += "Node: " + TEString::FromInt64(static_cast<int64_t>(node->ID)) + "|" + node->NodeType + "|" +
              node->Title + "|" + node->Category + "|" + TEString::FromFloat(node->Position.x) + " " +
              TEString::FromFloat(node->Position.y) + "|" + TEString::FromFloat(node->Size.x) + " " +
              TEString::FromFloat(node->Size.y) + "|" + TEString::FromFloat(node->HeaderColor.r) + " " +
              TEString::FromFloat(node->HeaderColor.g) + " " + TEString::FromFloat(node->HeaderColor.b) + " " +
              TEString::FromFloat(node->HeaderColor.a) + "\n";

        for (const auto &prop : node->Properties)
        {
            ss += "  Prop: " + prop.first + "=" + prop.second + "\n";
        }

        for (const auto &pin : node->InputPins)
        {
            const auto &col = pin.Type.GetColor();
            ss += "  InPin: " + TEString::FromInt64(static_cast<int64_t>(pin.ID)) + "|" + pin.Name + "|" +
                  TEString::FromInt((int)pin.Type.GetKind()) + "|" + pin.Type.GetTypeName() + "|" + pin.DefaultValue +
                  "|" + TEString::FromFloat(col.r) + " " + TEString::FromFloat(col.g) + " " +
                  TEString::FromFloat(col.b) + " " + TEString::FromFloat(col.a) + "\n";
        }

        for (const auto &pin : node->OutputPins)
        {
            const auto &col = pin.Type.GetColor();
            ss += "  OutPin: " + TEString::FromInt64(static_cast<int64_t>(pin.ID)) + "|" + pin.Name + "|" +
                  TEString::FromInt((int)pin.Type.GetKind()) + "|" + pin.Type.GetTypeName() + "|" + pin.DefaultValue +
                  "|" + TEString::FromFloat(col.r) + " " + TEString::FromFloat(col.g) + " " +
                  TEString::FromFloat(col.b) + " " + TEString::FromFloat(col.a) + "\n";
        }
    }

    const auto &connections = m_Graph.GetConnections();
    ss += "ConnectionsCount: " + TEString::FromInt(static_cast<int>(connections.Size())) + "\n";
    for (const auto &conn : connections)
    {
        ss += "Connection: " + TEString::FromInt64(static_cast<int64_t>(conn.ID)) + "|" +
              TEString::FromInt64(static_cast<int64_t>(conn.SourceNodeID)) + "|" +
              TEString::FromInt64(static_cast<int64_t>(conn.SourcePinID)) + "|" +
              TEString::FromInt64(static_cast<int64_t>(conn.TargetNodeID)) + "|" +
              TEString::FromInt64(static_cast<int64_t>(conn.TargetPinID)) + "\n";
    }

    outText = ss;
    return true;
}

bool GraphSerializer::Deserialize(const TEString &filepath, NodeFactoryFn factory)
{
    if (!TEFileSystem::Exists(filepath))
    {
        TE_CORE_ERROR("GraphSerializer: Failed to open file for reading at {0}", filepath);
        return false;
    }

    TEString text = TEFileSystem::ReadAllText(filepath);
    return DeserializeFromString(text, factory);
}

bool GraphSerializer::DeserializeFromString(const TEString &text, NodeFactoryFn factory)
{
    m_Graph.Clear();

    TERef<GraphNode> currentNode = nullptr;
    TEArray<TEString> lines = text.Split('\n');

    for (const auto &rawLine : lines)
    {
        TEString line = rawLine.Trim();
        if (line.IsEmpty() || line.StartsWith("#"))
            continue;

        if (line.StartsWith("Graph: "))
        {
            m_Graph.SetName(line.Mid(7));
        }
        else if (line.StartsWith("NextID: "))
        {
            uint64_t nextId = static_cast<uint64_t>(line.Mid(8).ToInt64());
            m_Graph.SetNextID(nextId);
        }
        else if (line.StartsWith("Node: "))
        {
            TEString content = line.Mid(6);
            auto parts = content.Split('|');
            if (parts.Num() >= 7)
            {
                uint64_t id = static_cast<uint64_t>(parts[0].ToInt64());
                TEString nodeType = parts[1];
                TEString title = parts[2];
                TEString category = parts[3];

                if (factory)
                    currentNode = factory(nodeType.c_str());
                if (!currentNode)
                    currentNode = CreateRef<GraphNode>();

                currentNode->ID = id;
                currentNode->NodeType = nodeType;
                currentNode->Title = title;
                currentNode->Category = category;

                auto posParts = parts[4].Split(' ');
                if (posParts.Num() >= 2)
                {
                    currentNode->Position.x = posParts[0].ToFloat();
                    currentNode->Position.y = posParts[1].ToFloat();
                }

                auto sizeParts = parts[5].Split(' ');
                if (sizeParts.Num() >= 2)
                {
                    currentNode->Size.x = sizeParts[0].ToFloat();
                    currentNode->Size.y = sizeParts[1].ToFloat();
                }

                auto colParts = parts[6].Split(' ');
                if (colParts.Num() >= 4)
                {
                    currentNode->HeaderColor.r = colParts[0].ToFloat();
                    currentNode->HeaderColor.g = colParts[1].ToFloat();
                    currentNode->HeaderColor.b = colParts[2].ToFloat();
                    currentNode->HeaderColor.a = colParts[3].ToFloat();
                }

                m_Graph.GetNodes().Add(currentNode);
            }
        }
        else if (line.StartsWith("Prop: ") && currentNode)
        {
            TEString content = line.Mid(6);
            int eqPos = content.Find("=");
            if (eqPos >= 0)
            {
                TEString key = content.Left(eqPos);
                TEString val = content.Mid(eqPos + 1);
                currentNode->SetProperty(key.c_str(), val.c_str());
            }
        }
        else if (line.StartsWith("InPin: ") && currentNode)
        {
            TEString content = line.Mid(7);
            auto parts = content.Split('|');
            if (parts.Num() >= 4)
            {
                uint64_t pinId = static_cast<uint64_t>(parts[0].ToInt64());
                TEString name = parts[1];
                GraphPinKind kind = (GraphPinKind)parts[2].ToInt();
                TEString typeName = parts[3];
                TEString defVal = parts.Num() >= 5 ? parts[4] : "";

                TEColor pinColor = TEColor::White();
                if (parts.Num() >= 6)
                {
                    auto cParts = parts[5].Split(' ');
                    if (cParts.Num() >= 4)
                    {
                        pinColor.r = cParts[0].ToFloat();
                        pinColor.g = cParts[1].ToFloat();
                        pinColor.b = cParts[2].ToFloat();
                        pinColor.a = cParts[3].ToFloat();
                    }
                }

                GraphPinType pinType(kind, typeName, pinColor);
                GraphPin pin(pinId, currentNode->ID, name, pinType, PinDirection::Input, defVal);
                currentNode->InputPins.Add(pin);
            }
        }
        else if (line.StartsWith("OutPin: ") && currentNode)
        {
            TEString content = line.Mid(8);
            auto parts = content.Split('|');
            if (parts.Num() >= 4)
            {
                uint64_t pinId = static_cast<uint64_t>(parts[0].ToInt64());
                TEString name = parts[1];
                GraphPinKind kind = (GraphPinKind)parts[2].ToInt();
                TEString typeName = parts[3];
                TEString defVal = parts.Num() >= 5 ? parts[4] : "";

                TEColor pinColor = TEColor::White();
                if (parts.Num() >= 6)
                {
                    auto cParts = parts[5].Split(' ');
                    if (cParts.Num() >= 4)
                    {
                        pinColor.r = cParts[0].ToFloat();
                        pinColor.g = cParts[1].ToFloat();
                        pinColor.b = cParts[2].ToFloat();
                        pinColor.a = cParts[3].ToFloat();
                    }
                }

                GraphPinType pinType(kind, typeName, pinColor);
                GraphPin pin(pinId, currentNode->ID, name, pinType, PinDirection::Output, defVal);
                currentNode->OutputPins.Add(pin);
            }
        }
        else if (line.StartsWith("Connection: "))
        {
            TEString content = line.Mid(12);
            auto parts = content.Split('|');
            if (parts.Num() >= 5)
            {
                uint64_t id = static_cast<uint64_t>(parts[0].ToInt64());
                uint64_t srcNode = static_cast<uint64_t>(parts[1].ToInt64());
                uint64_t srcPin = static_cast<uint64_t>(parts[2].ToInt64());
                uint64_t dstNode = static_cast<uint64_t>(parts[3].ToInt64());
                uint64_t dstPin = static_cast<uint64_t>(parts[4].ToInt64());

                GraphConnection conn(id, srcNode, srcPin, dstNode, dstPin);
                m_Graph.GetConnections().Add(conn);
            }
        }
    }

    return true;
}
