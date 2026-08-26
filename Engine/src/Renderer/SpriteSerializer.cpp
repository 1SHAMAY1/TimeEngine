#include "Core/PreRequisites.h"
#include "Renderer/SpriteSerializer.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

SpriteSerializer::SpriteSerializer(const TERef<Sprite> &sprite) : m_Sprite(sprite) {}

bool SpriteSerializer::Serialize(const TEString &filepath)
{
    if (!m_Sprite)
        return false;

    std::ofstream hout(filepath.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("SpriteSerializer: Failed to open file for writing at {0}", filepath.c_str());
        return false;
    }

    hout << "Sprite: " << m_Sprite->GetName().c_str() << "\n";
    hout << "TexturePath: " << m_Sprite->GetTexturePath().c_str() << "\n";

    float u0, v0, u1, v1;
    m_Sprite->GetUVs(u0, v0, u1, v1);
    hout << "UVs: " << u0 << " " << v0 << " " << u1 << " " << v1 << "\n";

    float px, py;
    m_Sprite->GetPivot(px, py);
    hout << "Pivot: " << px << " " << py << "\n";

    hout << "PixelsPerUnit: " << m_Sprite->GetPixelsPerUnit() << "\n";

    const auto &pts = m_Sprite->GetCustomColliderPoints();
    hout << "ColliderPoints: " << pts.Num();
    for (const auto &pt : pts)
    {
        hout << " " << pt.x << " " << pt.y;
    }
    hout << "\n";

    hout.close();
    return true;
}

bool SpriteSerializer::Deserialize(const TEString &filepath)
{
    if (!m_Sprite)
        return false;

    if (!TEFileSystem::Exists(filepath))
        return false;

    return TEFileSystem::ForEachLine(filepath,
                                     [this, &filepath](const TEString &line)
                                     {
                                         if (line.StartsWith("Sprite: "))
                                         {
                                             m_Sprite->SetName(line.Mid(8));
                                         }
                                         else if (line.StartsWith("TexturePath: "))
                                         {
                                             TEString texPath = line.Mid(13);
                                             if (!texPath.IsEmpty() && !TEFileSystem::Exists(texPath))
                                             {
                                                 TEString relativeTex = filepath.GetParentPath() / texPath;
                                                 if (TEFileSystem::Exists(relativeTex))
                                                     texPath = relativeTex;
                                             }
                                             m_Sprite->SetTexturePath(texPath);
                                         }
                                         else if (line.StartsWith("UVs: "))
                                         {
                                             TEArray<TEString> parts = line.Mid(5).Split(' ');
                                             if (parts.Num() >= 4)
                                             {
                                                 try
                                                 {
                                                     float u0 = std::stof(parts[0].c_str());
                                                     float v0 = std::stof(parts[1].c_str());
                                                     float u1 = std::stof(parts[2].c_str());
                                                     float v1 = std::stof(parts[3].c_str());
                                                     m_Sprite->SetUVs(u0, v0, u1, v1);
                                                 }
                                                 catch (...)
                                                 {
                                                 }
                                             }
                                         }
                                         else if (line.StartsWith("Pivot: "))
                                         {
                                             TEArray<TEString> parts = line.Mid(7).Split(' ');
                                             if (parts.Num() >= 2)
                                             {
                                                 try
                                                 {
                                                     float px = std::stof(parts[0].c_str());
                                                     float py = std::stof(parts[1].c_str());
                                                     m_Sprite->SetPivot(px, py);
                                                 }
                                                 catch (...)
                                                 {
                                                 }
                                             }
                                         }
                                         else if (line.StartsWith("PixelsPerUnit: "))
                                         {
                                             try
                                             {
                                                 float ppu = std::stof(line.Mid(15).Trim().c_str());
                                                 m_Sprite->SetPixelsPerUnit(ppu);
                                             }
                                             catch (...)
                                             {
                                             }
                                         }
                                         else if (line.StartsWith("ColliderPoints: "))
                                         {
                                             TEArray<TEString> parts = line.Mid(16).Split(' ');
                                             if (parts.Num() >= 1)
                                             {
                                                 try
                                                 {
                                                     size_t count = std::stoul(parts[0].c_str());
                                                     TEArray<TEVector2> pts;
                                                     size_t idx = 1;
                                                     for (size_t i = 0; i < count && idx + 1 < parts.Num(); ++i)
                                                     {
                                                         float x = std::stof(parts[idx++].c_str());
                                                         float y = std::stof(parts[idx++].c_str());
                                                         pts.Add({x, y});
                                                     }
                                                     m_Sprite->SetCustomColliderPoints(pts);
                                                 }
                                                 catch (...)
                                                 {
                                                 }
                                             }
                                         }
                                         return true;
                                     });
}
