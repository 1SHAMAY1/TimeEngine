#include "Renderer/Sprite.hpp"
#include "Core/Log.h"
#include "Renderer/SpriteSerializer.hpp"

namespace TE
{

void Sprite::OnContentBrowserCreate(const std::filesystem::path &path)
{
    std::filesystem::create_directories(path);
    std::string baseName = "NewSprite";
    std::filesystem::path finalPath = path / (baseName + ".tesprite");
    int counter = 1;
    while (std::filesystem::exists(finalPath))
    {
        finalPath = path / (baseName + "_" + std::to_string(counter++) + ".tesprite");
    }

    auto newSprite = std::make_shared<Sprite>();
    SpriteSerializer serializer(newSprite);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Sprite at {0}", finalPath.string());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create Sprite at {0}", finalPath.string());
    }
}

} // namespace TE
