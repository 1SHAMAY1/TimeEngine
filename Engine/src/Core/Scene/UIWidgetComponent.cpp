#include "Core/Scene/UIWidgetComponent.hpp"
#include "Core/Asset/UIAsset.hpp"
#include "Core/PreRequisites.h"

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
