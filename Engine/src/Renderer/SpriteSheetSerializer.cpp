#include "Core/PreRequisites.h"
#include "Renderer/SpriteSheetSerializer.hpp"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Utils/TEFileSystem.hpp"

SpriteSheetSerializer::SpriteSheetSerializer(const TERef<SpriteSheet> &spriteSheet) : m_SpriteSheet(spriteSheet) {}

bool SpriteSheetSerializer::Serialize(const TEString &filepath)
{
    if (!m_SpriteSheet)
        return false;

    TEString content = "SpriteSheet: " + m_SpriteSheet->GetName() + "\n";
    content += "TexturePath: " + m_SpriteSheet->GetTexturePath() + "\n";
    content += "CellWidth: " + TEString::FromInt64(static_cast<int64_t>(m_SpriteSheet->GetCellWidth())) + "\n";
    content += "CellHeight: " + TEString::FromInt64(static_cast<int64_t>(m_SpriteSheet->GetCellHeight())) + "\n";
    content += "PaddingX: " + TEString::FromInt64(static_cast<int64_t>(m_SpriteSheet->GetPaddingX())) + "\n";
    content += "PaddingY: " + TEString::FromInt64(static_cast<int64_t>(m_SpriteSheet->GetPaddingY())) + "\n";
    content += "OffsetX: " + TEString::FromInt64(static_cast<int64_t>(m_SpriteSheet->GetOffsetX())) + "\n";
    content += "OffsetY: " + TEString::FromInt64(static_cast<int64_t>(m_SpriteSheet->GetOffsetY())) + "\n";

    const auto &subFrames = m_SpriteSheet->GetSubFrames();
    content += "SubFrameCount: " + TEString::FromInt64(static_cast<int64_t>(subFrames.Num())) + "\n";
    for (const auto &f : subFrames)
    {
        content += "SubFrame: " + f.Name + "," + TEString::FromInt64(static_cast<int64_t>(f.Index)) + "," +
                   TEString::FromInt64(static_cast<int64_t>(f.X)) + "," +
                   TEString::FromInt64(static_cast<int64_t>(f.Y)) + "," +
                   TEString::FromInt64(static_cast<int64_t>(f.Width)) + "," +
                   TEString::FromInt64(static_cast<int64_t>(f.Height)) + "," + TEString::FromFloat(f.U0) + "," +
                   TEString::FromFloat(f.V0) + "," + TEString::FromFloat(f.U1) + "," + TEString::FromFloat(f.V1) + "\n";
    }

    const auto &anims = m_SpriteSheet->GetAnimations();
    content += "AnimCount: " + TEString::FromInt64(static_cast<int64_t>(anims.Num())) + "\n";
    for (const auto &a : anims)
    {
        content += "Anim: " + a.Name + "," + TEString::FromFloat(a.FPS) + "," + (a.Loop ? "1" : "0") + ",";
        for (size_t i = 0; i < a.FrameIndices.Num(); ++i)
        {
            content += TEString::FromInt64(static_cast<int64_t>(a.FrameIndices[i])) +
                       (i + 1 < a.FrameIndices.Num() ? ";" : "");
        }
        content += "\n";
    }

    return TEFileSystem::WriteAllText(filepath, content);
}

bool SpriteSheetSerializer::Deserialize(const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
        return false;

    TEString texPath = "";
    uint32_t cellW = 32, cellH = 32, padX = 0, padY = 0, offX = 0, offY = 0;
    TEArray<SubFrame> subFrames;
    TEArray<AnimSequence> anims;

    TEFileSystem::ForEachLine(filepath,
                              [&](const TEString &line)
                              {
                                  int colon = line.Find(":");
                                  if (colon < 0)
                                      return true;

                                  TEString key = line.Left(colon).Trim();
                                  TEString value = line.Mid(colon + 1).Trim();

                                  if (key == "TexturePath")
                                      texPath = value;
                                  else if (key == "CellWidth")
                                      cellW = (uint32_t)std::stoul(value.c_str());
                                  else if (key == "CellHeight")
                                      cellH = (uint32_t)std::stoul(value.c_str());
                                  else if (key == "PaddingX")
                                      padX = (uint32_t)std::stoul(value.c_str());
                                  else if (key == "PaddingY")
                                      padY = (uint32_t)std::stoul(value.c_str());
                                  else if (key == "OffsetX")
                                      offX = (uint32_t)std::stoul(value.c_str());
                                  else if (key == "OffsetY")
                                      offY = (uint32_t)std::stoul(value.c_str());
                                  else if (key == "SubFrame")
                                  {
                                      auto parts = value.Split(",");
                                      if (parts.Num() >= 10)
                                      {
                                          SubFrame f;
                                          f.Name = parts[0];
                                          f.Index = (uint32_t)std::stoul(parts[1].c_str());
                                          f.X = (uint32_t)std::stoul(parts[2].c_str());
                                          f.Y = (uint32_t)std::stoul(parts[3].c_str());
                                          f.Width = (uint32_t)std::stoul(parts[4].c_str());
                                          f.Height = (uint32_t)std::stoul(parts[5].c_str());
                                          f.U0 = std::stof(parts[6].c_str());
                                          f.V0 = std::stof(parts[7].c_str());
                                          f.U1 = std::stof(parts[8].c_str());
                                          f.V1 = std::stof(parts[9].c_str());
                                          subFrames.Add(f);
                                      }
                                  }
                                  else if (key == "Anim")
                                  {
                                      auto parts = value.Split(",");
                                      if (parts.Num() >= 3)
                                      {
                                          AnimSequence a;
                                          a.Name = parts[0];
                                          a.FPS = std::stof(parts[1].c_str());
                                          a.Loop = (parts[2] == "1" || parts[2] == "true");
                                          if (parts.Num() >= 4)
                                          {
                                              auto frameIndices = parts[3].Split(";");
                                              for (const auto &fIdxStr : frameIndices)
                                              {
                                                  if (!fIdxStr.IsEmpty())
                                                      a.FrameIndices.Add((uint32_t)std::stoul(fIdxStr.c_str()));
                                              }
                                          }
                                          anims.Add(a);
                                      }
                                  }
                                  return true;
                              });

    m_SpriteSheet->SetGridSettings(cellW, cellH, padX, padY, offX, offY);

    if (!texPath.IsEmpty())
    {
        TEString resolvedPath = texPath;
        if (!TEFileSystem::Exists(resolvedPath))
        {
            // 1. Check relative to .tespritesheet file directory
            TEString parentRel = filepath.GetParentPath() / texPath;
            if (TEFileSystem::Exists(parentRel))
            {
                resolvedPath = parentRel;
            }
            else
            {
                // 2. Check filename in same directory as .tespritesheet
                TEString fileInSameDir = filepath.GetParentPath() / texPath.GetFilename();
                if (TEFileSystem::Exists(fileInSameDir))
                {
                    resolvedPath = fileInSameDir;
                }
                else if (Project::GetActive())
                {
                    // 3. Check relative to active project Asset Directory
                    TEString assetDirRel = Project::GetAssetDirectory() / texPath;
                    if (TEFileSystem::Exists(assetDirRel))
                    {
                        resolvedPath = assetDirRel;
                    }
                }
            }
        }

        m_SpriteSheet->SetTexturePath(resolvedPath);
    }

    m_SpriteSheet->SetSubFrames(subFrames);
    m_SpriteSheet->SetAnimations(anims);

    if (m_SpriteSheet->GetSubFrames().IsEmpty() && m_SpriteSheet->GetTexture())
    {
        m_SpriteSheet->SliceGrid();
    }

    return true;
}
