#include "PreRequisites.h"
#include "PlayerControllerAsset.hpp"
#include "Log.h"
#include "Utils/TEFileSystem.hpp"

TE_REGISTER_ASSET(PlayerControllerAsset)

bool PlayerControllerAsset::LoadFromFile(const TEString &path)
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

                                  if (line.StartsWith("MappingContextPath: "))
                                  {
                                      m_MappingContextPath = line.Mid(20).Trim();
                                  }
                                  else if (line.StartsWith("EnableLookAtCursor: "))
                                  {
                                      m_bEnableLookAtCursor = (line.Mid(20).Trim() == "true");
                                  }
                                  else if (line.StartsWith("Deadzone: "))
                                  {
                                      m_Deadzone = std::stof(line.Mid(10).Trim().c_str());
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

bool PlayerControllerAsset::SaveToFile(const TEString &path)
{
    TEString content = "PlayerControllerAsset: " + m_Name + "\n";
    content += "MappingContextPath: " + m_MappingContextPath + "\n";
    content += "EnableLookAtCursor: " + TEString(m_bEnableLookAtCursor ? "true" : "false") + "\n";
    content += "Deadzone: " + TEString::FromFloat(m_Deadzone) + "\n";
    content += "---BeginTScript---\n";
    content += m_SourceText;
    if (!m_SourceText.EndsWith("\n"))
        content += "\n";
    content += "---EndTScript---\n";

    m_AssetPath = path;
    return TEFileSystem::WriteAllText(path, content);
}

void PlayerControllerAsset::OnContentBrowserCreate(const TEString &path)
{
    TEFileSystem::CreateDirectories(path);
    TEString baseName = "NewPlayerController";
    TEString finalPath = path / (baseName + ".tecontroller");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tecontroller");
    }

    auto pc = CreateRef<PlayerControllerAsset>();
    pc->SetName(finalPath.GetStem());
    pc->SaveToFile(finalPath);
}
