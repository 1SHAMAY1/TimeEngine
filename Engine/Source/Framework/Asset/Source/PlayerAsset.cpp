#include "PreRequisites.h"
#include "PlayerAsset.hpp"
#include "Log.h"
#include "Utils/TEFileSystem.hpp"

TE_REGISTER_ASSET(PlayerAsset)

bool PlayerAsset::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
        return false;

    m_AssetPath = path;
    m_Handle = AssetRegistry::RegisterPath(path);
    m_Name = path.GetStem();

    bool inSource = false;
    TEString sourceBuffer = "";

    TEFileSystem::ForEachLine(path,
                              [&](const TEString &line)
                              {
                                  if (inSource)
                                  {
                                      if (line == "---EndTScript---")
                                      {
                                          inSource = false;
                                          m_SourceText = sourceBuffer;
                                      }
                                      else
                                      {
                                          sourceBuffer += line + "\n";
                                      }
                                      return true;
                                  }

                                  if (line.StartsWith("SpritePath: "))
                                  {
                                      m_SpritePath = line.Mid(12).Trim();
                                  }
                                  else if (line.StartsWith("MovementMode: "))
                                  {
                                      TEString modeStr = line.Mid(14).Trim();
                                      m_MovementMode = (modeStr == "SideScroller")
                                                           ? EPlayerAssetMovementMode::SideScroller
                                                           : EPlayerAssetMovementMode::TopDown;
                                  }
                                  else if (line.StartsWith("MaxSpeed: "))
                                  {
                                      m_MaxSpeed = std::stof(line.Mid(10).Trim().c_str());
                                  }
                                  else if (line.StartsWith("Acceleration: "))
                                  {
                                      m_Acceleration = std::stof(line.Mid(14).Trim().c_str());
                                  }
                                  else if (line.StartsWith("Friction: "))
                                  {
                                      m_Friction = std::stof(line.Mid(10).Trim().c_str());
                                  }
                                  else if (line.StartsWith("JumpForce: "))
                                  {
                                      m_JumpForce = std::stof(line.Mid(11).Trim().c_str());
                                  }
                                  else if (line.StartsWith("ColliderSize: "))
                                  {
                                      auto parts = line.Mid(14).Trim().Split(' ');
                                      if (parts.Num() >= 2)
                                      {
                                          m_ColliderSize.x = std::stof(parts[0].c_str());
                                          m_ColliderSize.y = std::stof(parts[1].c_str());
                                      }
                                  }
                                  else if (line == "---BeginTScript---")
                                  {
                                      inSource = true;
                                      sourceBuffer = "";
                                  }
                                  return true;
                              });

    return true;
}

bool PlayerAsset::SaveToFile(const TEString &path)
{
    TEString content = "PlayerAsset: " + m_Name + "\n";
    content += "SpritePath: " + m_SpritePath + "\n";
    content += "MovementMode: " +
               TEString((m_MovementMode == EPlayerAssetMovementMode::SideScroller) ? "SideScroller" : "TopDown") + "\n";
    content += "MaxSpeed: " + TEString::FromFloat(m_MaxSpeed) + "\n";
    content += "Acceleration: " + TEString::FromFloat(m_Acceleration) + "\n";
    content += "Friction: " + TEString::FromFloat(m_Friction) + "\n";
    content += "JumpForce: " + TEString::FromFloat(m_JumpForce) + "\n";
    content +=
        "ColliderSize: " + TEString::FromFloat(m_ColliderSize.x) + " " + TEString::FromFloat(m_ColliderSize.y) + "\n";
    content += "---BeginTScript---\n";
    content += m_SourceText;
    if (!m_SourceText.EndsWith("\n"))
        content += "\n";
    content += "---EndTScript---\n";

    m_AssetPath = path;
    return TEFileSystem::WriteAllText(path, content);
}

void PlayerAsset::OnContentBrowserCreate(const TEString &path)
{
    TEFileSystem::CreateDirectories(path);
    TEString baseName = "NewPlayer";
    TEString finalPath = path / (baseName + ".teplayer");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".teplayer");
    }

    auto p = CreateRef<PlayerAsset>();
    p->SetName(finalPath.GetStem());
    p->SaveToFile(finalPath);
}
