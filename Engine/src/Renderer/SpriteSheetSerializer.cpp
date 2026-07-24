#include "Renderer/SpriteSheetSerializer.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE
{

SpriteSheetSerializer::SpriteSheetSerializer(const std::shared_ptr<SpriteSheet> &spriteSheet)
    : m_SpriteSheet(spriteSheet)
{
}

bool SpriteSheetSerializer::Serialize(const std::filesystem::path &filepath)
{
    std::ofstream hout(filepath);
    if (!hout.is_open())
    {
        TE_CORE_ERROR("SpriteSheetSerializer: Failed to open file for writing at {0}", filepath.string());
        return false;
    }

    hout << "SpriteSheet: " << m_SpriteSheet->GetName() << "\n";
    hout << "TexturePath: " << m_SpriteSheet->GetTexturePath() << "\n";
    hout << "CellWidth: " << m_SpriteSheet->GetCellWidth() << "\n";
    hout << "CellHeight: " << m_SpriteSheet->GetCellHeight() << "\n";
    hout << "PaddingX: " << m_SpriteSheet->GetPaddingX() << "\n";
    hout << "PaddingY: " << m_SpriteSheet->GetPaddingY() << "\n";
    hout << "OffsetX: " << m_SpriteSheet->GetOffsetX() << "\n";
    hout << "OffsetY: " << m_SpriteSheet->GetOffsetY() << "\n";

    const auto &subFrames = m_SpriteSheet->GetSubFrames();
    hout << "SubFrameCount: " << subFrames.size() << "\n";
    for (const auto &f : subFrames)
    {
        hout << "SubFrame: " << f.Name << "," << f.Index << "," << f.X << "," << f.Y << "," << f.Width << ","
             << f.Height << "," << f.U0 << "," << f.V0 << "," << f.U1 << "," << f.V1 << "\n";
    }

    const auto &anims = m_SpriteSheet->GetAnimations();
    hout << "AnimCount: " << anims.size() << "\n";
    for (const auto &a : anims)
    {
        hout << "Anim: " << a.Name << "," << a.FPS << "," << (a.Loop ? "1" : "0") << ",";
        for (size_t i = 0; i < a.FrameIndices.size(); ++i)
        {
            hout << a.FrameIndices[i] << (i + 1 < a.FrameIndices.size() ? ";" : "");
        }
        hout << "\n";
    }

    hout.close();
    return true;
}

bool SpriteSheetSerializer::Deserialize(const std::filesystem::path &filepath)
{
    std::ifstream hin(filepath);
    if (!hin.is_open())
        return false;

    std::string line;
    std::string texPath = "";
    uint32_t cellW = 32, cellH = 32, padX = 0, padY = 0, offX = 0, offY = 0;
    std::vector<SubFrame> subFrames;
    std::vector<AnimSequence> anims;

    while (std::getline(hin, line))
    {
        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);

        if (key == "TexturePath")
            texPath = value;
        else if (key == "CellWidth")
            cellW = (uint32_t)std::stoul(value);
        else if (key == "CellHeight")
            cellH = (uint32_t)std::stoul(value);
        else if (key == "PaddingX")
            padX = (uint32_t)std::stoul(value);
        else if (key == "PaddingY")
            padY = (uint32_t)std::stoul(value);
        else if (key == "OffsetX")
            offX = (uint32_t)std::stoul(value);
        else if (key == "OffsetY")
            offY = (uint32_t)std::stoul(value);
        else if (key == "SubFrame")
        {
            std::stringstream ss(value);
            std::string item;
            SubFrame f;
            if (std::getline(ss, f.Name, ',') && std::getline(ss, item, ','))
            {
                f.Index = (uint32_t)std::stoul(item);
                if (std::getline(ss, item, ',')) f.X = (uint32_t)std::stoul(item);
                if (std::getline(ss, item, ',')) f.Y = (uint32_t)std::stoul(item);
                if (std::getline(ss, item, ',')) f.Width = (uint32_t)std::stoul(item);
                if (std::getline(ss, item, ',')) f.Height = (uint32_t)std::stoul(item);
                if (std::getline(ss, item, ',')) f.U0 = std::stof(item);
                if (std::getline(ss, item, ',')) f.V0 = std::stof(item);
                if (std::getline(ss, item, ',')) f.U1 = std::stof(item);
                if (std::getline(ss, item, ',')) f.V1 = std::stof(item);
                subFrames.push_back(f);
            }
        }
        else if (key == "Anim")
        {
            std::stringstream ss(value);
            std::string item;
            AnimSequence a;
            if (std::getline(ss, a.Name, ',') && std::getline(ss, item, ','))
            {
                a.FPS = std::stof(item);
                if (std::getline(ss, item, ',')) a.Loop = (item == "1" || item == "true");
                if (std::getline(ss, item, ','))
                {
                    std::stringstream framesSS(item);
                    std::string fIdxStr;
                    while (std::getline(framesSS, fIdxStr, ';'))
                    {
                        if (!fIdxStr.empty())
                            a.FrameIndices.push_back((uint32_t)std::stoul(fIdxStr));
                    }
                }
                anims.push_back(a);
            }
        }
    }

    hin.close();

    m_SpriteSheet->SetGridSettings(cellW, cellH, padX, padY, offX, offY);

    if (!texPath.empty())
    {
        std::filesystem::path resolvedPath = texPath;
        if (!std::filesystem::exists(resolvedPath))
        {
            resolvedPath = filepath.parent_path() / texPath;
        }

        if (std::filesystem::exists(resolvedPath))
        {
            m_SpriteSheet->SetTexturePath(resolvedPath.string());
        }
    }

    m_SpriteSheet->SetSubFrames(subFrames);
    m_SpriteSheet->SetAnimations(anims);

    if (m_SpriteSheet->GetSubFrames().empty() && m_SpriteSheet->GetTexture())
    {
        m_SpriteSheet->SliceGrid();
    }

    return true;
}

} // namespace TE
