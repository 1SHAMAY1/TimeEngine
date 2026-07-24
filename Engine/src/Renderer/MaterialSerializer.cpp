#include "Renderer/MaterialSerializer.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE
{

MaterialSerializer::MaterialSerializer(const std::shared_ptr<Material> &material) : m_Material(material) {}

bool MaterialSerializer::Serialize(const std::filesystem::path &filepath)
{
    std::ofstream hout(filepath);
    if (!hout.is_open())
    {
        TE_CORE_ERROR("MaterialSerializer: Failed to open file for writing at {0}", filepath.string());
        return false;
    }

    hout << "Material: " << m_Material->GetName() << "\n";

    auto color = m_Material->GetColor().GetValue();
    hout << "Color: " << color.r << " " << color.g << " " << color.b << " " << color.a << "\n";

    const auto &stack = m_Material->GetPassStack();
    hout << "PassStackCount: " << stack.size() << "\n";
    for (const auto &node : stack)
    {
        hout << "Node: " << node.Name << "|" << (int)node.Type << "|" << (node.Enabled ? 1 : 0) << "|"
             << node.TexturePath << "|" << node.Color.x << " " << node.Color.y << " " << node.Color.z << " "
             << node.Color.w << "|" << node.FloatVal1 << "|" << node.FloatVal2 << "|" << node.FloatVal3 << "|"
             << node.FloatVal4 << "|" << node.BlendMode << "|" << node.TargetQueueIndex << "|" << node.QueueName
             << "\n";
    }

    hout.close();
    return true;
}

bool MaterialSerializer::Deserialize(const std::filesystem::path &filepath)
{
    std::ifstream hin(filepath);
    if (!hin.is_open())
        return false;

    auto &stack = m_Material->GetPassStack();
    stack.clear();

    std::string line;
    while (std::getline(hin, line))
    {
        if (line.find("Material: ") == 0)
        {
            m_Material->SetName(line.substr(10));
        }
        else if (line.find("Color: ") == 0)
        {
            std::stringstream ss(line.substr(7));
            float r, g, b, a;
            ss >> r >> g >> b >> a;
            m_Material->SetColor(TEColor(r, g, b, a));
        }
        else if (line.find("Node: ") == 0)
        {
            std::string content = line.substr(6);
            std::stringstream ss(content);
            std::string part;
            std::vector<std::string> parts;
            while (std::getline(ss, part, '|'))
            {
                parts.push_back(part);
            }

            if (parts.size() >= 10)
            {
                MaterialPassNode node;
                node.Name = parts[0];
                node.Type = (MaterialPassNodeType)std::stoi(parts[1]);
                node.Enabled = (std::stoi(parts[2]) != 0);
                node.TexturePath = parts[3];
                if (!node.TexturePath.empty() && std::filesystem::exists(node.TexturePath))
                {
                    node.TextureRef = std::make_shared<Texture>(node.TexturePath);
                }

                std::stringstream colorSS(parts[4]);
                colorSS >> node.Color.x >> node.Color.y >> node.Color.z >> node.Color.w;

                node.FloatVal1 = std::stof(parts[5]);
                node.FloatVal2 = std::stof(parts[6]);
                node.FloatVal3 = std::stof(parts[7]);
                node.FloatVal4 = std::stof(parts[8]);
                node.BlendMode = std::stoi(parts[9]);

                if (parts.size() >= 12)
                {
                    node.TargetQueueIndex = std::stoi(parts[10]);
                    node.QueueName = parts[11];
                }

                stack.push_back(node);
            }
        }
    }

    if (stack.empty())
    {
        // Re-add default nodes if none deserialized
        MaterialPassNode baseNode;
        baseNode.Name = "Base Surface Slab";
        baseNode.Type = MaterialPassNodeType::BaseSurfaceSlab;
        baseNode.Enabled = true;
        stack.push_back(baseNode);
    }

    hin.close();
    return true;
}

} // namespace TE
