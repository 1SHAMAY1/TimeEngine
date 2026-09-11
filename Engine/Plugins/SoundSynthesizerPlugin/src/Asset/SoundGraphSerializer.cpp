#include "SoundGraphSerializer.hpp"
#include "../Graph/SoundNodeRegistry.hpp"
#include "Core/Log.h"
#include <fstream>

namespace SoundStudio
{

bool SoundGraphSerializer::Serialize(TERef<SoundGraph> graph, const TEString &filepath)
{
    if (!graph)
        return false;

    std::ofstream out(filepath.c_str());
    if (!out.is_open())
        return false;

    out << "SoundGraph: 1.0\n";
    out << "Nodes:\n";

    for (const auto &pair : graph->GetNodes())
    {
        auto node = pair.second;
        out << "  - ID: " << node->GetID() << "\n";
        out << "    Type: " << node->GetNodeTypeName().c_str() << "\n";
        out << "    PosX: " << node->GetPosition().x << "\n";
        out << "    PosY: " << node->GetPosition().y << "\n";
    }

    out << "Connections:\n";
    for (const auto &conn : graph->GetConnections())
    {
        out << "  - OutPin: " << conn.OutputPinID << "\n";
        out << "    InPin: " << conn.InputPinID << "\n";
    }

    out.close();
    return true;
}

bool SoundGraphSerializer::Deserialize(TERef<SoundGraph> graph, const TEString &filepath)
{
    if (!graph)
        return false;

    std::ifstream in(filepath.c_str());
    if (!in.is_open())
        return false;

    TEString content;
    in.seekg(0, std::ios::end);
    size_t size = (size_t)in.tellg();
    in.seekg(0, std::ios::beg);
    content.Reserve(size + 1);
    in.read(content.Data(), size);
    content.Data()[size] = '\0';
    content.SyncFromBuffer();

    TEArray<TEString> lines = content.Split('\n');
    for (size_t i = 0; i < lines.Size(); ++i)
    {
        TEString line = lines[i].Trim();
        if (line.IsEmpty() || line.StartsWith("#"))
            continue;
    }

    return true;
}

} // namespace SoundStudio
