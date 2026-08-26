#include "Renderer/TextureSerializer.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <sstream>

TextureSerializer::TextureSerializer(const TERef<Texture> &texture) : m_Texture(texture) {}

bool TextureSerializer::Serialize(const TEString &filepath)
{
    std::ofstream hout(filepath.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("TextureSerializer: Failed to open file for writing at {0}", filepath.c_str());
        return false;
    }

    TEString filterStr = (m_Texture->GetFilterMode() == TextureFilterMode::Nearest) ? "Nearest" : "Linear";
    TEString wrapStr = "Repeat";
    if (m_Texture->GetWrapMode() == TextureWrapMode::ClampToEdge)
        wrapStr = "ClampToEdge";
    else if (m_Texture->GetWrapMode() == TextureWrapMode::MirroredRepeat)
        wrapStr = "MirroredRepeat";

    hout << "Texture2D: " << m_Texture->GetName().c_str() << "\n";
    hout << "ImagePath: " << m_Texture->GetFilePath().c_str() << "\n";
    hout << "Width: " << m_Texture->GetWidth() << "\n";
    hout << "Height: " << m_Texture->GetHeight() << "\n";
    hout << "Channels: " << m_Texture->GetChannels() << "\n";
    hout << "FilterMode: " << filterStr.c_str() << "\n";
    hout << "WrapMode: " << wrapStr.c_str() << "\n";
    hout << "GenerateMipmaps: " << (m_Texture->GetGenerateMipmaps() ? "true" : "false") << "\n";
    hout << "PremultipliedAlpha: " << (m_Texture->GetPremultipliedAlpha() ? "true" : "false") << "\n";

    hout.close();
    return true;
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
            resolvedPath = filepath.GetParentPath() / imagePath;
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
