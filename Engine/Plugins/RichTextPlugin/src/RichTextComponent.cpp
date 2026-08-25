#include "RichTextComponent.hpp"
#include "Utils/TimeGUI.hpp"


void RichTextComponent::OnInitialize()
{
    TComponent::OnInitialize();
    RecomputeLayout();
}

void RichTextComponent::SetMarkupText(const TEString &text)
{
    MarkupText = text;
    m_bDirty = true;
}

void RichTextComponent::SetStyleTable(AssetHandle handle)
{
    StyleTableHandle = handle;
    m_bDirty = true;
}

void RichTextComponent::SetFontAsset(AssetHandle handle)
{
    FontAssetHandle = handle;
    m_bDirty = true;
}

void RichTextComponent::RecomputeLayout()
{
    auto styleTable = AssetManager::GetAsset<RichTextTable>(StyleTableHandle);
    auto fontAsset = AssetManager::GetAsset<FontAsset>(FontAssetHandle);

    TEArray<RichTextSpan> spans = RichTextParser::Parse(MarkupText, styleTable);
    if (fontAsset)
    {
        m_Layout = RichTextLayoutEngine::ComputeLayout(spans, fontAsset, WrapWidth);
    }
    else
    {
        m_Layout = RichTextLayoutResult{};
    }

    m_AnimatedLayout = m_Layout;
    m_bDirty = false;
}

void RichTextComponent::Tick(float deltaTime)
{
    if (m_bDirty)
    {
        RecomputeLayout();
    }

    if (EnableAnimations)
    {
        m_ElapsedTime += deltaTime;

        if (TypewriterSpeed > 0.0f && TypewriterProgress < 1.0f)
        {
            TypewriterProgress += deltaTime * TypewriterSpeed;
            if (TypewriterProgress > 1.0f)
                TypewriterProgress = 1.0f;
        }

        m_AnimatedLayout = m_Layout;
        RichTextAnimator::AnimateLayout(m_AnimatedLayout, m_ElapsedTime, deltaTime, TypewriterProgress);
    }
}

TEString RichTextComponent::HitTest(const TEVector2 &localPoint) const
{
    return RichTextLayoutEngine::HitTestLink(m_Layout, localPoint);
}

void RichTextComponent::OnDrawInspector()
{
    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "RichText Component");
    TimeGUI::Separator();

    if (TimeGUI::InputTextMultiline("Markup Text", MarkupText, TEVector2(-1.0f, 80.0f)))
    {
        m_bDirty = true;
    }

    if (TimeGUI::DragFloat("Wrap Width", &WrapWidth, 5.0f, 0.0f, 2000.0f, "%.0f px"))
    {
        m_bDirty = true;
    }

    TimeGUI::Checkbox("Enable Animations", &EnableAnimations);
    TimeGUI::SameLine();
    TimeGUI::SliderFloat("Typewriter", &TypewriterProgress, 0.0f, 1.0f, "%.2f");

    if (TimeGUI::Button("Recompute Layout"))
    {
        RecomputeLayout();
    }

    TimeGUI::TextDisabled("Glyphs: %zu | Links: %zu | Bounds: %.0f x %.0f",
                          m_Layout.GlyphQuads.Num(), m_Layout.LinkHitboxes.Num(),
                          m_Layout.Bounds.x, m_Layout.Bounds.y);
}

