#pragma once

#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "RichTextAnimator.hpp"
#include "RichTextLayout.hpp"
#include "RichTextParser.hpp"
#include "RichTextTable.hpp"
#include "RichTextTypes.hpp"
#include "Utils/TEString.hpp"


class RichTextComponent : public TComponent
{
public:
    TEString MarkupText = "<color=gold><b>Hello</b></color> <wave>TimeEngine Rich Text!</wave>";
    AssetHandle StyleTableHandle = 0;
    AssetHandle FontAssetHandle = 0;
    float WrapWidth = 0.0f;
    ERichTextAlign Alignment = ERichTextAlign::Left;

    bool EnableAnimations = true;
    float TypewriterProgress = 1.0f;
    float TypewriterSpeed = 0.0f;

    virtual ~RichTextComponent() override = default;

    virtual void OnInitialize() override;
    virtual void Tick(float deltaTime) override;
    virtual void OnDrawInspector() override;

    void SetMarkupText(const TEString &text);
    const TEString &GetMarkupText() const { return MarkupText; }

    void SetStyleTable(AssetHandle handle);
    void SetFontAsset(AssetHandle handle);

    void RecomputeLayout();
    const RichTextLayoutResult &GetLayout() const { return m_Layout; }

    TEString HitTest(const TEVector2 &localPoint) const;

private:
    float m_ElapsedTime = 0.0f;
    bool m_bDirty = true;
    RichTextLayoutResult m_Layout;
    RichTextLayoutResult m_AnimatedLayout;
};

