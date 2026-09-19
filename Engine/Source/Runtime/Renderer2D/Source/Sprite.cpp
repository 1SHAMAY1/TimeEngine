#include "PreRequisites.h"
#include "Sprite.hpp"
#include "Log.h"
#include "SpriteSerializer.hpp"
#include "Texture.hpp"
#include "Utils/TEFileSystem.hpp"

#include "Project/Project.hpp"

TE_REGISTER_ASSET(Sprite)

bool Sprite::LoadFromFile(const TEString &path)
{
    auto self = TERef<Sprite>(this, [](Sprite *) {});
    SpriteSerializer serializer(self);
    return serializer.Deserialize(path);
}

void Sprite::SetTexturePath(const TEString &path)
{
    m_TexturePath = path.LexicallyNormal();
    if (m_TexturePath.empty())
    {
        m_Texture.reset();
        return;
    }

    TEString resolved = m_TexturePath;
    if (!TEFileSystem::Exists(resolved))
    {
        if (Project::GetActive())
        {
            TEString assetRel = (Project::GetAssetDirectory() / m_TexturePath).LexicallyNormal();
            if (TEFileSystem::Exists(assetRel))
                resolved = assetRel;
            else
            {
                TEString projRel = (Project::GetProjectDirectory() / m_TexturePath).LexicallyNormal();
                if (TEFileSystem::Exists(projRel))
                    resolved = projRel;
            }
        }
    }

    if (TEFileSystem::Exists(resolved))
    {
        m_Texture = CreateRef<Texture>(resolved);
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
