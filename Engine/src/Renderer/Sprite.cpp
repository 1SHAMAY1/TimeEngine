#include "Renderer/Sprite.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Renderer/SpriteSerializer.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/TEFileSystem.hpp"

#include "Utils/TEFileSystem.hpp"
TE_REGISTER_ASSET(Sprite)

bool Sprite::LoadFromFile(const TEString &path)
{
    auto self = TERef<Sprite>(this, [](Sprite *) {});
    SpriteSerializer serializer(self);
    return serializer.Deserialize(path);
}

void Sprite::SetTexturePath(const TEString &path)
{
    m_TexturePath = path;
    if (!path.empty() && TEFileSystem::Exists(path))
    {
        m_Texture = CreateRef<Texture>(path);
    }
}

void Sprite::GenerateAutoContourCollider(float alphaThreshold)
{
    // Generate default box contour points if no custom outline algorithm provided
    m_CustomColliderPoints.Empty();
    m_CustomColliderPoints.Add({-0.5f, -0.5f});
    m_CustomColliderPoints.Add({0.5f, -0.5f});
    m_CustomColliderPoints.Add({0.5f, 0.5f});
    m_CustomColliderPoints.Add({-0.5f, 0.5f});
}

void Sprite::OnContentBrowserCreate(const TEString &path)
{
    TEFileSystem::CreateDirectories(path);
    TEString baseName = "NewSprite";
    TEString finalPath = path / (baseName + ".tesprite");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tesprite");
    }

    auto newSprite = CreateRef<Sprite>();
    SpriteSerializer serializer(newSprite);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Sprite at {0}", finalPath.c_str());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create Sprite at {0}", finalPath.c_str());
    }
}
