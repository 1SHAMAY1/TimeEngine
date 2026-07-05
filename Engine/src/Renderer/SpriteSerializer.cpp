#include "Renderer/SpriteSerializer.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE
{

SpriteSerializer::SpriteSerializer(const std::shared_ptr<Sprite> &sprite) : m_Sprite(sprite) {}

bool SpriteSerializer::Serialize(const std::filesystem::path &filepath)
{
    std::ofstream hout(filepath);
    if (!hout.is_open())
    {
        TE_CORE_ERROR("SpriteSerializer: Failed to open file for writing at {0}", filepath.string());
        return false;
    }

    hout << "Sprite: " << m_Sprite->GetName() << "\n";
    hout << "Texture: 0\n"; // 0 implies none or default
    hout << "UVs: 0 0 1 1\n";

    hout.close();
    return true;
}

bool SpriteSerializer::Deserialize(const std::filesystem::path &filepath)
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
