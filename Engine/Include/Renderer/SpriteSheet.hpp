#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetManager.hpp"


struct SubFrame
{
    TEString Name;
    uint32_t Index = 0;
    uint32_t X = 0;
    uint32_t Y = 0;
    uint32_t Width = 32;
    uint32_t Height = 32;
    float U0 = 0.0f;
    float V0 = 0.0f;
    float U1 = 1.0f;
    float V1 = 1.0f;
};

struct AnimSequence
{
    TEString Name = "Default";
    float FPS = 12.0f;
    bool Loop = true;
    TEArray<uint32_t> FrameIndices;
};

class SpriteSheet : public Asset
{
public:
    SpriteSheet() = default;
    virtual ~SpriteSheet() = default;

    const TEString &GetTexturePath() const { return m_TexturePath; }
    void SetTexturePath(const TEString &path);

    TERef<class Texture> GetTexture() const { return m_Texture; }

    uint32_t GetCellWidth() const { return m_CellWidth; }
    uint32_t GetCellHeight() const { return m_CellHeight; }
    uint32_t GetPaddingX() const { return m_PaddingX; }
    uint32_t GetPaddingY() const { return m_PaddingY; }
    uint32_t GetOffsetX() const { return m_OffsetX; }
    uint32_t GetOffsetY() const { return m_OffsetY; }

    void SetGridSettings(uint32_t cellW, uint32_t cellH, uint32_t padX = 0, uint32_t padY = 0, uint32_t offX = 0,
                         uint32_t offY = 0);

    const TEArray<SubFrame> &GetSubFrames() const { return m_SubFrames; }
    TEArray<SubFrame> &GetSubFrames() { return m_SubFrames; }
    void SetSubFrames(const TEArray<SubFrame> &frames) { m_SubFrames = frames; }

    const TEArray<AnimSequence> &GetAnimations() const { return m_Animations; }
    TEArray<AnimSequence> &GetAnimations() { return m_Animations; }
    void SetAnimations(const TEArray<AnimSequence> &anims) { m_Animations = anims; }

    void SliceGrid();
    void SliceAutoAlpha(float alphaThreshold = 0.05f);

    void AddAnimation(const TEString &name);
    void RemoveAnimation(size_t index);

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString type = "SpriteSheet";
        return type;
    }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual void SetName(const TEString &name) { m_Name = name; }
    virtual const TEString &GetHoverDescription() const override { return m_TexturePath; }

    virtual TEString GetDefaultExtension() const override { return ".tespritesheet"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/SpriteSheetIcon.png"; }

    virtual TERef<Asset> Clone() const override { return CreateRef<SpriteSheet>(); }
    virtual bool LoadFromFile(const TEString &path) override;

    virtual void OnContentBrowserCreate(const TEString &path) override;

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "New Sprite Sheet";
    TEString m_TexturePath;
    TERef<class Texture> m_Texture;

    uint32_t m_CellWidth = 32;
    uint32_t m_CellHeight = 32;
    uint32_t m_PaddingX = 0;
    uint32_t m_PaddingY = 0;
    uint32_t m_OffsetX = 0;
    uint32_t m_OffsetY = 0;

    TEArray<SubFrame> m_SubFrames;
    TEArray<AnimSequence> m_Animations;
};

