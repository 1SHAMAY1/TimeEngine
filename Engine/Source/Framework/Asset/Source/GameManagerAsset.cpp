#include "PreRequisites.h"
#include "GameManagerAsset.hpp"
#include "Log.h"
#include "Utils/TEFileSystem.hpp"

TE_REGISTER_ASSET(GameManagerAsset)

bool GameManagerAsset::LoadFromFile(const TEString &path)
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

                                  if (line.StartsWith("PlayerAsset: "))
                                  {
                                      m_PlayerAssetPath = line.Mid(13).Trim();
                                  }
                                  else if (line.StartsWith("ControllerAsset: "))
                                  {
                                      m_ControllerAssetPath = line.Mid(17).Trim();
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

bool GameManagerAsset::SaveToFile(const TEString &path)
{
    TEString content = "GameManagerAsset: " + m_Name + "\n";
    content += "PlayerAsset: " + m_PlayerAssetPath + "\n";
    content += "ControllerAsset: " + m_ControllerAssetPath + "\n";
    content += "---BeginTScript---\n";
    content += m_SourceText;
    if (!m_SourceText.EndsWith("\n"))
        content += "\n";
    content += "---EndTScript---\n";

    m_AssetPath = path;
    return TEFileSystem::WriteAllText(path, content);
}

void GameManagerAsset::OnContentBrowserCreate(const TEString &path)
{
    TEFileSystem::CreateDirectories(path);
    TEString baseName = "NewGameManager";
    TEString finalPath = path / (baseName + ".tegamemanager");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tegamemanager");
    }

    auto gm = CreateRef<GameManagerAsset>();
    gm->SetName(finalPath.GetStem());
    gm->SaveToFile(finalPath);
}
