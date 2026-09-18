#include "PreRequisites.h"
#include "UIWidgetComponent.hpp"
#include "UIAsset.hpp"

void UIWidgetComponent::OnAttach()
{
    if (!UIAssetPath.empty())
    {
        LoadFromUIAsset(UIAssetPath);
    }
}

void UIWidgetComponent::Tick(float deltaTime)
{
    if (m_RootWidget && Visible)
    {
        m_RootWidget->OnUpdate(deltaTime);
    }
}

bool UIWidgetComponent::LoadFromUIAsset(const TEString &path)
{
    UIAsset asset;
    if (asset.LoadFromFile(path))
    {
        m_RootWidget = asset.InstantiateWidgetTree();
        if (m_RootWidget)
        {
            m_RootWidget->SetSize(DrawSize);
        }
        return true;
    }
    return false;
}

#include "Renderer2D.hpp"

void UIWidgetComponent::OnRender(Renderer2D *renderer, const TEMatrix4 &worldModel,
                                const TERef<Material> &material) const
{
    if (!Visible || !renderer)
        return;

    TEVector4 centerWorld = worldModel * TEVector4(0.0f, 0.0f, 0.0f, 1.0f);
    renderer->SubmitRectOutline(TEVector2(centerWorld.x, centerWorld.y), DrawSize, 1.0f,
                                TEColor(0.2f, 0.8f, 1.0f, 0.5f));
}
