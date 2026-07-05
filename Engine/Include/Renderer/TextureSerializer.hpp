#pragma once
#include "Renderer/Texture.hpp"
#include <filesystem>
#include <memory>

namespace TE
{

class TextureSerializer
{
public:
    TextureSerializer(const std::shared_ptr<Texture> &texture);

    bool Serialize(const std::filesystem::path &filepath);
    bool Deserialize(const std::filesystem::path &filepath);

private:
    std::shared_ptr<Texture> m_Texture;
};

} // namespace TE
