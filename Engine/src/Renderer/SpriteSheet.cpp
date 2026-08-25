#include "Core/PreRequisites.h"
#include "Renderer/SpriteSheet.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"
#include "Renderer/SpriteSheetSerializer.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Renderer/Texture.hpp"


#include "Utils/TEFileSystem.hpp"
TE_REGISTER_ASSET(SpriteSheet)

bool SpriteSheet::LoadFromFile(const TEString &path)
{
    auto self = TERef<SpriteSheet>(this, [](SpriteSheet*){});
    SpriteSheetSerializer serializer(self);
    return serializer.Deserialize(path);
}

void SpriteSheet::SetTexturePath(const TEString &path)
{
    m_TexturePath = path;
    if (!path.empty() && TEFileSystem::Exists(path))
    {
        m_Texture = CreateRef<Texture>(path);
    }
}

void SpriteSheet::SetGridSettings(uint32_t cellW, uint32_t cellH, uint32_t padX, uint32_t padY, uint32_t offX,
                                  uint32_t offY)
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
    m_SubFrames.Empty();

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
            frame.Name = "Frame_" + TEString::FromInt(index);
            frame.X = x;
            frame.Y = y;
            frame.Width = m_CellWidth;
            frame.Height = m_CellHeight;

            frame.U0 = (float)x / (float)texW;
            frame.V0 = (float)y / (float)texH;
            frame.U1 = (float)(x + m_CellWidth) / (float)texW;
            frame.V1 = (float)(y + m_CellHeight) / (float)texH;

            m_SubFrames.Add(frame);
            index++;
        }
    }
}

void SpriteSheet::SliceAutoAlpha(float alphaThreshold)
{
    // Re-slice grid as baseline
    SliceGrid();
}

void SpriteSheet::AddAnimation(const TEString &name)
{
    AnimSequence anim;
    anim.Name = name.empty() ? ("Anim_" + TEString::FromInt64(static_cast<int64_t>(m_Animations.Num()))) : name;
    anim.FPS = 12.0f;
    anim.Loop = true;
    for (size_t i = 0; i < m_SubFrames.Num(); ++i)
    {
        anim.FrameIndices.Add((uint32_t)i);
    }
    m_Animations.Add(anim);
}

void SpriteSheet::RemoveAnimation(size_t index)
{
    if (index < m_Animations.Num())
    {
        m_Animations.RemoveAt(index);
    }
}

void SpriteSheet::OnContentBrowserCreate(const TEString &path)
{
    TEFileSystem::CreateDirectories(path);
    TEString baseName = "NewSpriteSheet";
    TEString finalPath = path / (baseName + ".tesheet");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tesheet");
    }

    auto newSpriteSheet = CreateRef<SpriteSheet>();
    newSpriteSheet->SetName(finalPath.GetStem());
    SpriteSheetSerializer serializer(newSpriteSheet);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New SpriteSheet at {0}", finalPath.c_str());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create SpriteSheet at {0}", finalPath.c_str());
    }
}

