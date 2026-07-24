#include "Renderer/TextureSerializer.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE
{

TextureSerializer::TextureSerializer(const std::shared_ptr<Texture> &texture) : m_Texture(texture) {}

bool TextureSerializer::Serialize(const std::filesystem::path &filepath)
{
    std::ofstream hout(filepath);
    if (!hout.is_open())
    {
        TE_CORE_ERROR("TextureSerializer: Failed to open file for writing at {0}", filepath.string());
        return false;
    }

    std::string filterStr = (m_Texture->GetFilterMode() == TextureFilterMode::Nearest) ? "Nearest" : "Linear";
    std::string wrapStr = "Repeat";
    if (m_Texture->GetWrapMode() == TextureWrapMode::ClampToEdge)
        wrapStr = "ClampToEdge";
    else if (m_Texture->GetWrapMode() == TextureWrapMode::MirroredRepeat)
        wrapStr = "MirroredRepeat";

    hout << "Texture2D: " << m_Texture->GetName() << "\n";
    hout << "ImagePath: " << m_Texture->GetFilePath() << "\n";
    hout << "Width: " << m_Texture->GetWidth() << "\n";
    hout << "Height: " << m_Texture->GetHeight() << "\n";
    hout << "Channels: " << m_Texture->GetChannels() << "\n";
    hout << "FilterMode: " << filterStr << "\n";
    hout << "WrapMode: " << wrapStr << "\n";
    hout << "GenerateMipmaps: " << (m_Texture->GetGenerateMipmaps() ? "true" : "false") << "\n";
    hout << "PremultipliedAlpha: " << (m_Texture->GetPremultipliedAlpha() ? "true" : "false") << "\n";

    hout.close();
    return true;
}

bool TextureSerializer::Deserialize(const std::filesystem::path &filepath)
{
    std::ifstream hin(filepath);
    if (!hin.is_open())
        return false;

    std::string line;
    std::string imagePath = "";
    TextureFilterMode filter = TextureFilterMode::Linear;
    TextureWrapMode wrap = TextureWrapMode::Repeat;
    bool mipmaps = false;
    bool premultAlpha = false;

    while (std::getline(hin, line))
    {
        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Trim leading spaces
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);

        if (key == "ImagePath")
        {
            imagePath = value;
        }
        else if (key == "FilterMode")
        {
            filter = (value == "Nearest") ? TextureFilterMode::Nearest : TextureFilterMode::Linear;
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
    }

    hin.close();

    if (!imagePath.empty())
    {
        std::filesystem::path resolvedPath = imagePath;
        if (!std::filesystem::exists(resolvedPath))
        {
            resolvedPath = filepath.parent_path() / imagePath;
        }

        if (std::filesystem::exists(resolvedPath))
        {
            m_Texture->LoadImageSource(resolvedPath.string());
        }
    }

    m_Texture->SetFilterMode(filter);
    m_Texture->SetWrapMode(wrap);
    m_Texture->SetGenerateMipmaps(mipmaps);
    m_Texture->SetPremultipliedAlpha(premultAlpha);

    return true;
}

} // namespace TE
