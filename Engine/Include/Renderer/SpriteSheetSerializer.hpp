#pragma once
#include "Renderer/SpriteSheet.hpp"
#include <filesystem>
#include <memory>

namespace TE
{

class SpriteSheetSerializer
{
public:
    SpriteSheetSerializer(const std::shared_ptr<SpriteSheet> &spriteSheet);

    bool Serialize(const std::filesystem::path &filepath);
    bool Deserialize(const std::filesystem::path &filepath);

private:
    std::shared_ptr<SpriteSheet> m_SpriteSheet;
};

} // namespace TE
