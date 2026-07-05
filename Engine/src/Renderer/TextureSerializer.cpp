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

    hout << "Texture2D: " << m_Texture->GetName() << "\n";
    hout << "ImagePath: blank.png\n";
    hout << "FilterMode: Linear\n";
    hout << "WrapMode: Repeat\n";

    hout.close();
    return true;
}

bool TextureSerializer::Deserialize(const std::filesystem::path &filepath)
{
    std::ifstream hin(filepath);
    if (!hin.is_open())
        return false;

    std::string line;
    while (std::getline(hin, line))
    {
        // Add deserialization when properties are fully implemented
    }

    hin.close();
    return true;
}

} // namespace TE
