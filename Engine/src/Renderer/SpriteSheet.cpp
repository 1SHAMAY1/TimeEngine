#include "Renderer/SpriteSheet.hpp"
#include "Core/Log.h"
#include "Renderer/SpriteSheetSerializer.hpp"
#include "Renderer/Texture.hpp"

namespace TE
{

void SpriteSheet::SetTexturePath(const std::string &path)
{
    m_TexturePath = path;
    if (!path.empty() && std::filesystem::exists(path))
    {
        m_Texture = std::make_shared<Texture>(path);
    }
}

void SpriteSheet::SetGridSettings(uint32_t cellW, uint32_t cellH, uint32_t padX, uint32_t padY, uint32_t offX, uint32_t offY)
{
    m_CellWidth = (cellW > 0) ? cellW : 32;
    m_CellHeight = (cellH > 0) ? cellH : 32;
    m_PaddingX = padX;
    m_PaddingY = padY;
    m_OffsetX = offX;
    m_OffsetY = offY;
}

void SpriteSheet::SliceGrid()
{
    m_SubFrames.clear();

    if (!m_Texture || m_Texture->GetWidth() == 0 || m_Texture->GetHeight() == 0)
        return;

    uint32_t texW = m_Texture->GetWidth();
    uint32_t texH = m_Texture->GetHeight();

    uint32_t index = 0;
    for (uint32_t y = m_OffsetY; y + m_CellHeight <= texH; y += (m_CellHeight + m_PaddingY))
    {
        for (uint32_t x = m_OffsetX; x + m_CellWidth <= texW; x += (m_CellWidth + m_PaddingX))
        {
            SubFrame frame;
            frame.Index = index;
            frame.Name = "Frame_" + std::to_string(index);
            frame.X = x;
            frame.Y = y;
            frame.Width = m_CellWidth;
            frame.Height = m_CellHeight;

            frame.U0 = (float)x / (float)texW;
            frame.V0 = (float)y / (float)texH;
            frame.U1 = (float)(x + m_CellWidth) / (float)texW;
            frame.V1 = (float)(y + m_CellHeight) / (float)texH;

            m_SubFrames.push_back(frame);
            index++;
        }
    }
}

void SpriteSheet::SliceAutoAlpha(float alphaThreshold)
{
    // Re-slice grid as baseline
    SliceGrid();
}

void SpriteSheet::AddAnimation(const std::string &name)
{
    AnimSequence anim;
    anim.Name = name.empty() ? ("Anim_" + std::to_string(m_Animations.size())) : name;
    anim.FPS = 12.0f;
    anim.Loop = true;
    for (size_t i = 0; i < m_SubFrames.size(); ++i)
    {
        anim.FrameIndices.push_back((uint32_t)i);
    }
    m_Animations.push_back(anim);
}

void SpriteSheet::RemoveAnimation(size_t index)
{
    if (index < m_Animations.size())
    {
        m_Animations.erase(m_Animations.begin() + index);
    }
}

void SpriteSheet::OnContentBrowserCreate(const std::filesystem::path &path)
{
    std::filesystem::create_directories(path);
    std::string baseName = "NewSpriteSheet";
    std::filesystem::path finalPath = path / (baseName + ".tesheet");
    int counter = 1;
    while (std::filesystem::exists(finalPath))
    {
        finalPath = path / (baseName + "_" + std::to_string(counter++) + ".tesheet");
    }

    auto newSpriteSheet = std::make_shared<SpriteSheet>();
    newSpriteSheet->SetName(finalPath.stem().string());
    SpriteSheetSerializer serializer(newSpriteSheet);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New SpriteSheet at {0}", finalPath.string());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create SpriteSheet at {0}", finalPath.string());
    }
}

} // namespace TE
