#include "Renderer/Sprite.hpp"
#include "Core/Log.h"
#include "Renderer/SpriteSerializer.hpp"
#include "Renderer/Texture.hpp"

namespace TE
{

void Sprite::SetTexturePath(const std::string &path)
{
    m_TexturePath = path;
    if (!path.empty() && std::filesystem::exists(path))
    {
        m_Texture = std::make_shared<Texture>(path);
    }
}

void Sprite::GenerateAutoContourCollider(float alphaThreshold)
{
    // Generate default box contour points if no custom outline algorithm provided
    m_CustomColliderPoints.clear();
    m_CustomColliderPoints.push_back({-0.5f, -0.5f});
    m_CustomColliderPoints.push_back({0.5f, -0.5f});
    m_CustomColliderPoints.push_back({0.5f, 0.5f});
    m_CustomColliderPoints.push_back({-0.5f, 0.5f});
}

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
