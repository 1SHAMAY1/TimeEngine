#include "Renderer/SpriteSheet.hpp"
#include "Core/Log.h"
#include "Renderer/SpriteSheetSerializer.hpp"

namespace TE
{

void SpriteSheet::OnContentBrowserCreate(const std::filesystem::path &path)
{
    std::filesystem::create_directories(path);
    std::string baseName = "NewSpriteSheet";
    std::filesystem::path finalPath = path / (baseName + ".tespritesheet");
    int counter = 1;
    while (std::filesystem::exists(finalPath))
    {
        finalPath = path / (baseName + "_" + std::to_string(counter++) + ".tespritesheet");
    }

    auto newSpriteSheet = std::make_shared<SpriteSheet>();
    SpriteSheetSerializer serializer(newSpriteSheet);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New SpriteSheet at {0}", finalPath.string());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create SpriteSheet at {0}", finalPath.string());
    }
}

} // namespace TE
