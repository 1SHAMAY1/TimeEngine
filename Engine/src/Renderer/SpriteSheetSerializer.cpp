#include "Renderer/SpriteSheetSerializer.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE
{

SpriteSheetSerializer::SpriteSheetSerializer(const std::shared_ptr<SpriteSheet> &spriteSheet)
    : m_SpriteSheet(spriteSheet)
{
}

bool SpriteSheetSerializer::Serialize(const std::filesystem::path &filepath)
{
    std::ofstream hout(filepath);
    if (!hout.is_open())
    {
        TE_CORE_ERROR("SpriteSheetSerializer: Failed to open file for writing at {0}", filepath.string());
        return false;
    }

    hout << "SpriteSheet: " << m_SpriteSheet->GetName() << "\n";
    hout << "Texture: 0\n";
    hout << "TileWidth: 32\n";
    hout << "TileHeight: 32\n";
    hout << "Spacing: 0\n";
    hout << "Margin: 0\n";

    hout.close();
    return true;
}

bool SpriteSheetSerializer::Deserialize(const std::filesystem::path &filepath)
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
