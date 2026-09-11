#include "Core/PreRequisites.h"
#include "Renderer/SpriteSerializer.hpp"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Utils/TEFileSystem.hpp"

SpriteSerializer::SpriteSerializer(const TERef<Sprite> &sprite) : m_Sprite(sprite) {}

bool SpriteSerializer::Serialize(const TEString &filepath)
{
    if (!m_Sprite)
        return false;

    TEString content = "Sprite: " + m_Sprite->GetName() + "\n";
    content += "TexturePath: " + m_Sprite->GetTexturePath() + "\n";

    float u0, v0, u1, v1;
    m_Sprite->GetUVs(u0, v0, u1, v1);
    content += "UVs: " + TEString::FromFloat(u0) + " " + TEString::FromFloat(v0) + " " + TEString::FromFloat(u1) + " " +
               TEString::FromFloat(v1) + "\n";

    float px, py;
    m_Sprite->GetPivot(px, py);
    content += "Pivot: " + TEString::FromFloat(px) + " " + TEString::FromFloat(py) + "\n";

    content += "PixelsPerUnit: " + TEString::FromFloat(m_Sprite->GetPixelsPerUnit()) + "\n";

    const auto &pts = m_Sprite->GetCustomColliderPoints();
    content += "ColliderPoints: " + TEString::FromInt64(static_cast<int64_t>(pts.Num()));
    for (const auto &pt : pts)
    {
        content += " " + TEString::FromFloat(pt.x) + " " + TEString::FromFloat(pt.y);
    }
    content += "\n";

    return TEFileSystem::WriteAllText(filepath, content);
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
                                             m_Sprite->SetName(line.Mid(8).Trim());
                                         }
                                         else if (line.StartsWith("TexturePath: "))
                                         {
                                             TEString texPath = line.Mid(13).Trim();
                                             if (!texPath.IsEmpty())
                                             {
                                                 TEString resolvedPath = texPath;
                                                 if (!TEFileSystem::Exists(resolvedPath))
                                                 {
                                                     // 1. Check relative to .tesprite file directory
                                                     TEString parentRel = filepath.GetParentPath() / texPath;
                                                     if (TEFileSystem::Exists(parentRel))
                                                     {
                                                         resolvedPath = parentRel;
                                                     }
                                                     else
                                                     {
                                                         // 2. Check filename in same directory as .tesprite
                                                         TEString fileInSameDir =
                                                             filepath.GetParentPath() / texPath.GetFilename();
                                                         if (TEFileSystem::Exists(fileInSameDir))
                                                         {
                                                             resolvedPath = fileInSameDir;
                                                         }
                                                         else if (Project::GetActive())
                                                         {
                                                             // 3. Check relative to active project Asset Directory
                                                             TEString assetDirRel =
                                                                 Project::GetAssetDirectory() / texPath;
                                                             if (TEFileSystem::Exists(assetDirRel))
                                                             {
                                                                 resolvedPath = assetDirRel;
                                                             }
                                                         }
                                                     }
                                                 }
                                                 m_Sprite->SetTexturePath(resolvedPath);
                                             }
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
