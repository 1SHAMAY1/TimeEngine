#include "Renderer/MaterialSerializer.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <sstream>

#include "Utils/TEFileSystem.hpp"
MaterialSerializer::MaterialSerializer(const TERef<Material> &material) : m_Material(material) {}

bool MaterialSerializer::Serialize(const TEString &filepath)
{
    std::ofstream hout(filepath.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("MaterialSerializer: Failed to open file for writing at {0}", filepath.c_str());
        return false;
    }

    hout << "Material: " << m_Material->GetName() << "\n";

    auto color = m_Material->GetColor().GetValue();
    hout << "Color: " << color.r << " " << color.g << " " << color.b << " " << color.a << "\n";

    const auto &stack = m_Material->GetPassStack();
    hout << "PassStackCount: " << stack.Num() << "\n";
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

bool MaterialSerializer::Deserialize(const TEString &filepath)
{
    auto &stack = m_Material->GetPassStack();
    stack.Empty();

    if (!TEFileSystem::Exists(filepath))
        return false;

    TEFileSystem::ForEachLine(filepath,
                              [&](const TEString &line) -> bool
                              {
                                  if (line.find("Material: ") == 0)
                                  {
                                      m_Material->SetName(line.substr(10));
                                  }
                                  else if (line.find("Color: ") == 0)
                                  {
                                      TEArray<TEString> colorParts = line.substr(7).Split(' ');
                                      float r = colorParts.Num() > 0 ? colorParts[0].ToFloat() : 0.0f;
                                      float g = colorParts.Num() > 1 ? colorParts[1].ToFloat() : 0.0f;
                                      float b = colorParts.Num() > 2 ? colorParts[2].ToFloat() : 0.0f;
                                      float a = colorParts.Num() > 3 ? colorParts[3].ToFloat() : 1.0f;
                                      m_Material->SetColor(TEColor(r, g, b, a));
                                  }
                                  else if (line.find("Node: ") == 0)
                                  {
                                      TEString content = line.substr(6);
                                      TEArray<TEString> parts = content.Split('|');

                                      if (parts.Num() >= 10)
                                      {
                                          MaterialPassNode node;
                                          node.Name = parts[0];
                                          node.Type = (MaterialPassNodeType)std::stoi(parts[1]);
                                          node.Enabled = (std::stoi(parts[2]) != 0);
                                          node.TexturePath = parts[3];
                                          if (!node.TexturePath.empty() && TEFileSystem::Exists(node.TexturePath))
                                          {
                                              node.TextureRef = CreateRef<Texture>(node.TexturePath);
                                          }

                                          TEArray<TEString> colorParts = parts[4].Split(' ');
                                          node.Color.x = colorParts.Num() > 0 ? colorParts[0].ToFloat() : 0.0f;
                                          node.Color.y = colorParts.Num() > 1 ? colorParts[1].ToFloat() : 0.0f;
                                          node.Color.z = colorParts.Num() > 2 ? colorParts[2].ToFloat() : 0.0f;
                                          node.Color.w = colorParts.Num() > 3 ? colorParts[3].ToFloat() : 1.0f;

                                          node.FloatVal1 = std::stof(parts[5]);
                                          node.FloatVal2 = std::stof(parts[6]);
                                          node.FloatVal3 = std::stof(parts[7]);
                                          node.FloatVal4 = std::stof(parts[8]);
                                          node.BlendMode = std::stoi(parts[9]);

                                          if (parts.Num() >= 12)
                                          {
                                              node.TargetQueueIndex = std::stoi(parts[10]);
                                              node.QueueName = parts[11];
                                          }

                                          stack.Add(node);
                                      }
                                  }
                                  return true;
                              });

    if (stack.IsEmpty())
    {
        // Re-add default nodes if none deserialized
        MaterialPassNode baseNode;
        baseNode.Name = "Base Surface Slab";
        baseNode.Type = MaterialPassNodeType::BaseSurfaceSlab;
        baseNode.Enabled = true;
        stack.Add(baseNode);
    }

    return true;
}
