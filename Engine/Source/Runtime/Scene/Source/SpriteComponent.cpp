#include "PreRequisites.h"
#include "SpriteComponent.hpp"
#include "AssetManager.hpp"
#include "Material.hpp"
#include "Renderer2D.hpp"
#include "Texture.hpp"

void SpriteComponent::OnRender(Renderer2D *renderer, const TEMatrix4 &worldModel, const TERef<Material> &material) const
{
    if (!renderer)
        return;

    TERef<Texture> tex = TextureRef;
    if (!tex && TextureHandle != 0)
    {
        tex = AssetManager::GetAsset<Texture>(TextureHandle);
    }
    if (!tex && !TexturePath.empty())
    {
        AssetHandle h = AssetManager::LoadAsset(TexturePath);
        if (h != 0)
        {
            tex = AssetManager::GetAsset<Texture>(h);
        }
        if (!tex)
        {
            tex = CreateRef<Texture>(TexturePath);
        }
    }

    if (tex)
    {
        tex->Bind(0);
    }

    auto renderMat = material ? material : Material::GetDefault();
    renderer->SubmitQuad(worldModel, Color, renderMat);
}
