#include "PreRequisites.h"
#include "AnimatedSpriteComponent.hpp"
#include "AssetManager.hpp"
#include "SpriteSheet.hpp"
#include "Texture.hpp"

AnimatedSpriteComponent::AnimatedSpriteComponent()
{
    AnimationClip defaultClip;
    defaultClip.Name = "Idle";
    defaultClip.FrameDuration = 0.1f;
    defaultClip.Loop = true;
    m_Clips.Add("Idle", defaultClip);
}

void AnimatedSpriteComponent::OnAttach()
{
    if (AutoPlay)
    {
        Play(CurrentClipName);
    }
}

void AnimatedSpriteComponent::Tick(float deltaTime)
{
    if (!m_IsPlaying || PlaybackSpeed <= 0.0f)
        return;

    const AnimationClip *clip = GetClip(CurrentClipName);
    if (!clip || clip->FrameDuration <= 0.0001f)
        return;

    size_t totalFrames = clip->FrameIndices.Num();
    if (totalFrames <= 1)
        return;

    m_FrameTimer += deltaTime * PlaybackSpeed;
    if (m_FrameTimer >= clip->FrameDuration)
    {
        m_FrameTimer -= clip->FrameDuration;

        if (clip->PingPong)
        {
            if (m_IsReversing)
            {
                if (m_CurrentFrameIndex > 0)
                {
                    m_CurrentFrameIndex--;
                }
                else
                {
                    m_IsReversing = false;
                    m_CurrentFrameIndex = 1;
                }
            }
            else
            {
                if (m_CurrentFrameIndex < static_cast<int>(totalFrames) - 1)
                {
                    m_CurrentFrameIndex++;
                }
                else
                {
                    m_IsReversing = true;
                    m_CurrentFrameIndex = static_cast<int>(totalFrames) - 2;
                }
            }
        }
        else
        {
            m_CurrentFrameIndex++;
            if (m_CurrentFrameIndex >= static_cast<int>(totalFrames))
            {
                if (clip->Loop)
                {
                    m_CurrentFrameIndex = 0;
                }
                else
                {
                    m_CurrentFrameIndex = static_cast<int>(totalFrames) - 1;
                    m_IsPlaying = false;
                }
            }
        }
    }
}

void AnimatedSpriteComponent::Play(const TEString &clipName)
{
    CurrentClipName = clipName;
    m_CurrentFrameIndex = 0;
    m_FrameTimer = 0.0f;
    m_IsPlaying = true;
    m_IsReversing = false;
}

void AnimatedSpriteComponent::Pause() { m_IsPlaying = false; }

void AnimatedSpriteComponent::Resume() { m_IsPlaying = true; }

void AnimatedSpriteComponent::Stop()
{
    m_IsPlaying = false;
    m_CurrentFrameIndex = 0;
    m_FrameTimer = 0.0f;
}

void AnimatedSpriteComponent::AddClip(const AnimationClip &clip) { m_Clips.Add(clip.Name, clip); }

const AnimationClip *AnimatedSpriteComponent::GetClip(const TEString &name) const { return m_Clips.Find(name); }

TEVector4 AnimatedSpriteComponent::GetCurrentUVRect() const
{
    float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;

    TERef<SpriteSheet> sheet = nullptr;
    if (SpriteSheetHandle != 0)
    {
        sheet = AssetManager::GetAsset<SpriteSheet>(SpriteSheetHandle);
    }
    if (!sheet && !SpriteSheetPath.empty())
    {
        AssetHandle h = AssetManager::LoadAsset(SpriteSheetPath);
        if (h != 0)
        {
            sheet = AssetManager::GetAsset<SpriteSheet>(h);
        }
        if (!sheet)
        {
            sheet = CreateRef<SpriteSheet>();
            sheet->LoadFromFile(SpriteSheetPath);
        }
    }

    if (sheet)
    {
        const auto &subFrames = sheet->GetSubFrames();
        const AnimationClip *clip = GetClip(CurrentClipName);
        int frameIdx = m_CurrentFrameIndex;

        if (clip && !clip->FrameIndices.empty())
        {
            if (m_CurrentFrameIndex >= 0 && m_CurrentFrameIndex < static_cast<int>(clip->FrameIndices.size()))
            {
                frameIdx = clip->FrameIndices[m_CurrentFrameIndex];
            }
        }

        if (frameIdx >= 0 && frameIdx < static_cast<int>(subFrames.size()))
        {
            const auto &sf = subFrames[frameIdx];
            u0 = sf.U0;
            v0 = sf.V0;
            u1 = sf.U1;
            v1 = sf.V1;
        }
    }

    if (FlipX)
        std::swap(u0, u1);
    if (FlipY)
        std::swap(v0, v1);
    return {u0, v0, u1, v1};
}

#include "Renderer2D.hpp"

void AnimatedSpriteComponent::OnRender(Renderer2D *renderer, const TEMatrix4 &worldModel,
                                       const TERef<Material> &material) const
{
    if (!renderer)
        return;

    TERef<SpriteSheet> sheet = nullptr;
    if (SpriteSheetHandle != 0)
    {
        sheet = AssetManager::GetAsset<SpriteSheet>(SpriteSheetHandle);
    }
    if (!sheet && !SpriteSheetPath.empty())
    {
        AssetHandle h = AssetManager::LoadAsset(SpriteSheetPath);
        if (h != 0)
        {
            sheet = AssetManager::GetAsset<SpriteSheet>(h);
        }
        if (!sheet)
        {
            sheet = CreateRef<SpriteSheet>();
            sheet->LoadFromFile(SpriteSheetPath);
        }
    }

    TERef<Texture> tex = sheet ? sheet->GetTexture() : nullptr;
    if (tex)
    {
        tex->Bind(0);
    }

    auto renderMat = material ? material : Material::GetDefault();
    renderer->SubmitQuad(worldModel, Color, renderMat);
}


