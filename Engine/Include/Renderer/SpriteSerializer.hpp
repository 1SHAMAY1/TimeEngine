#pragma once
#include "Renderer/Sprite.hpp"
#include <filesystem>
#include <memory>

namespace TE
{

class SpriteSerializer
{
public:
    SpriteSerializer(const std::shared_ptr<Sprite> &sprite);

    bool Serialize(const std::filesystem::path &filepath);
    bool Deserialize(const std::filesystem::path &filepath);

private:
    std::shared_ptr<Sprite> m_Sprite;
};

} // namespace TE
