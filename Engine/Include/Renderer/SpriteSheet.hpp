#pragma once
#include "Core/Asset/Asset.hpp"

namespace TE
{

struct SubFrame
{
    std::string Name;
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
    std::string Name = "Default";
    float FPS = 12.0f;
    bool Loop = true;
    std::vector<uint32_t> FrameIndices;
};

class SpriteSheet : public Asset
{
public:
    SpriteSheet() = default;
    virtual ~SpriteSheet() = default;

    const std::string &GetTexturePath() const { return m_TexturePath; }
    void SetTexturePath(const std::string &path);

    std::shared_ptr<class Texture> GetTexture() const { return m_Texture; }

    uint32_t GetCellWidth() const { return m_CellWidth; }
    uint32_t GetCellHeight() const { return m_CellHeight; }
    uint32_t GetPaddingX() const { return m_PaddingX; }
    uint32_t GetPaddingY() const { return m_PaddingY; }
    uint32_t GetOffsetX() const { return m_OffsetX; }
    uint32_t GetOffsetY() const { return m_OffsetY; }

    void SetGridSettings(uint32_t cellW, uint32_t cellH, uint32_t padX = 0, uint32_t padY = 0, uint32_t offX = 0,
                         uint32_t offY = 0);

    const std::vector<SubFrame> &GetSubFrames() const { return m_SubFrames; }
    std::vector<SubFrame> &GetSubFrames() { return m_SubFrames; }
    void SetSubFrames(const std::vector<SubFrame> &frames) { m_SubFrames = frames; }

    const std::vector<AnimSequence> &GetAnimations() const { return m_Animations; }
    std::vector<AnimSequence> &GetAnimations() { return m_Animations; }
    void SetAnimations(const std::vector<AnimSequence> &anims) { m_Animations = anims; }

    void SliceGrid();
    void SliceAutoAlpha(float alphaThreshold = 0.05f);

    void AddAnimation(const std::string &name);
    void RemoveAnimation(size_t index);

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string &GetType() const override
    {
        static std::string type = "SpriteSheet";
        return type;
    }
    virtual const std::string &GetName() const override { return m_Name; }
    virtual void SetName(const std::string &name) { m_Name = name; }
    virtual const std::string &GetHoverDescription() const override { return m_TexturePath; }

    virtual std::string GetDefaultExtension() const override { return ".tespritesheet"; }
    virtual std::string GetDefaultIconPath() const override { return "Resources/Editor/SpriteSheetIcon.png"; }

    virtual void OnContentBrowserCreate(const std::filesystem::path &path) override;

private:
    AssetHandle m_Handle = 0;
    std::string m_Name = "New Sprite Sheet";
    std::string m_TexturePath;
    std::shared_ptr<class Texture> m_Texture;

    uint32_t m_CellWidth = 32;
    uint32_t m_CellHeight = 32;
    uint32_t m_PaddingX = 0;
    uint32_t m_PaddingY = 0;
    uint32_t m_OffsetX = 0;
    uint32_t m_OffsetY = 0;

    std::vector<SubFrame> m_SubFrames;
    std::vector<AnimSequence> m_Animations;
};

} // namespace TE
