#include "Core/PreRequisites.h"
#include "Renderer/TextureSerializer.hpp"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Utils/TEFileSystem.hpp"

TextureSerializer::TextureSerializer(const TERef<Texture> &texture) : m_Texture(texture) {}

bool TextureSerializer::Serialize(const TEString &filepath)
{
    if (!m_Texture)
        return false;

    TEString filterStr = (m_Texture->GetFilterMode() == TextureFilterMode::Nearest) ? "Nearest" : "Linear";
    TEString wrapStr = "Repeat";
    if (m_Texture->GetWrapMode() == TextureWrapMode::ClampToEdge)
        wrapStr = "ClampToEdge";
    else if (m_Texture->GetWrapMode() == TextureWrapMode::MirroredRepeat)
        wrapStr = "MirroredRepeat";

    TEString content = "Texture2D: " + m_Texture->GetName() + "\n";
    content += "ImagePath: " + m_Texture->GetFilePath() + "\n";
    content += "Width: " + TEString::FromInt64(static_cast<int64_t>(m_Texture->GetWidth())) + "\n";
    content += "Height: " + TEString::FromInt64(static_cast<int64_t>(m_Texture->GetHeight())) + "\n";
    content += "Channels: " + TEString::FromInt64(static_cast<int64_t>(m_Texture->GetChannels())) + "\n";
    content += "FilterMode: " + filterStr + "\n";
    content += "WrapMode: " + wrapStr + "\n";
    content += "GenerateMipmaps: " + TEString(m_Texture->GetGenerateMipmaps() ? "true" : "false") + "\n";
    content += "PremultipliedAlpha: " + TEString(m_Texture->GetPremultipliedAlpha() ? "true" : "false") + "\n";

    return TEFileSystem::WriteAllText(filepath, content);
}

bool TextureSerializer::Deserialize(const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
        return false;

    TEString imagePath = "";
    TextureFilterMode filter = TextureFilterMode::Linear;
    TextureWrapMode wrap = TextureWrapMode::Repeat;
    bool mipmaps = false;
    bool premultAlpha = false;

    TEFileSystem::ForEachLine(filepath,
                              [&](const TEString &line)
                              {
                                  int colon = line.Find(":");
                                  if (colon < 0)
                                      return true;

                                  TEString key = line.Left(colon).Trim();
                                  TEString value = line.Mid(colon + 1).Trim();

                                  if (key == "ImagePath")
                                  {
                                      imagePath = value;
                                  }
                                  else if (key == "FilterMode")
                                  {
                                      filter =
                                          (value == "Nearest") ? TextureFilterMode::Nearest : TextureFilterMode::Linear;
                                  }
                                  else if (key == "WrapMode")
                                  {
                                      if (value == "ClampToEdge")
                                          wrap = TextureWrapMode::ClampToEdge;
                                      else if (value == "MirroredRepeat")
                                          wrap = TextureWrapMode::MirroredRepeat;
                                      else
                                          wrap = TextureWrapMode::Repeat;
                                  }
                                  else if (key == "GenerateMipmaps")
                                  {
                                      mipmaps = (value == "true" || value == "1");
                                  }
                                  else if (key == "PremultipliedAlpha")
                                  {
                                      premultAlpha = (value == "true" || value == "1");
                                  }
                                  return true;
                              });

    if (!imagePath.IsEmpty())
    {
        TEString resolvedPath = imagePath;
        if (!TEFileSystem::Exists(resolvedPath))
        {
            // 1. Check relative to .tetexture file directory
            TEString parentRel = filepath.GetParentPath() / imagePath;
            if (TEFileSystem::Exists(parentRel))
            {
                resolvedPath = parentRel;
            }
            else
            {
                // 2. Check filename in same directory as .tetexture
                TEString fileInSameDir = filepath.GetParentPath() / imagePath.GetFilename();
                if (TEFileSystem::Exists(fileInSameDir))
                {
                    resolvedPath = fileInSameDir;
                }
                else if (Project::GetActive())
                {
                    // 3. Check relative to active project Asset Directory
                    TEString assetDirRel = Project::GetAssetDirectory() / imagePath;
                    if (TEFileSystem::Exists(assetDirRel))
                    {
                        resolvedPath = assetDirRel;
                    }
                }
            }
        }

        if (TEFileSystem::Exists(resolvedPath))
        {
            m_Texture->LoadImageSource(resolvedPath);
        }
    }

    m_Texture->SetFilterMode(filter);
    m_Texture->SetWrapMode(wrap);
    m_Texture->SetGenerateMipmaps(mipmaps);
    m_Texture->SetPremultipliedAlpha(premultAlpha);

    return true;
}
