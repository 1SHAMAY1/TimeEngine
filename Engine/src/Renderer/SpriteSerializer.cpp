#include "Renderer/SpriteSerializer.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE
{

SpriteSerializer::SpriteSerializer(const std::shared_ptr<Sprite> &sprite) : m_Sprite(sprite) {}

bool SpriteSerializer::Serialize(const std::filesystem::path &filepath)
{
    if (!m_Sprite)
        return false;

    std::ofstream hout(filepath);
    if (!hout.is_open())
    {
        TE_CORE_ERROR("SpriteSerializer: Failed to open file for writing at {0}", filepath.string());
        return false;
    }

    hout << "Sprite: " << m_Sprite->GetName() << "\n";
    hout << "TexturePath: " << m_Sprite->GetTexturePath() << "\n";

    float u0, v0, u1, v1;
    m_Sprite->GetUVs(u0, v0, u1, v1);
    hout << "UVs: " << u0 << " " << v0 << " " << u1 << " " << v1 << "\n";

    float px, py;
    m_Sprite->GetPivot(px, py);
    hout << "Pivot: " << px << " " << py << "\n";

    hout << "PixelsPerUnit: " << m_Sprite->GetPixelsPerUnit() << "\n";

    const auto &pts = m_Sprite->GetCustomColliderPoints();
    hout << "ColliderPoints: " << pts.size();
    for (const auto &pt : pts)
    {
        hout << " " << pt.x << " " << pt.y;
    }
    hout << "\n";

    hout.close();
    return true;
}

bool SpriteSerializer::Deserialize(const std::filesystem::path &filepath)
{
    if (!m_Sprite)
        return false;

    std::ifstream hin(filepath);
    if (!hin.is_open())
        return false;

    std::string line;
    while (std::getline(hin, line))
    {
        if (line.rfind("Sprite: ", 0) == 0)
        {
            m_Sprite->SetName(line.substr(8));
        }
        else if (line.rfind("TexturePath: ", 0) == 0)
        {
            std::string texPath = line.substr(13);
            if (!texPath.empty() && !std::filesystem::exists(texPath))
            {
                std::filesystem::path relativeTex = filepath.parent_path() / texPath;
                if (std::filesystem::exists(relativeTex))
                    texPath = relativeTex.string();
            }
            m_Sprite->SetTexturePath(texPath);
        }
        else if (line.rfind("UVs: ", 0) == 0)
        {
            std::stringstream ss(line.substr(5));
            float u0, v0, u1, v1;
            if (ss >> u0 >> v0 >> u1 >> v1)
            {
                m_Sprite->SetUVs(u0, v0, u1, v1);
            }
        }
        else if (line.rfind("Pivot: ", 0) == 0)
        {
            std::stringstream ss(line.substr(7));
            float px, py;
            if (ss >> px >> py)
            {
                m_Sprite->SetPivot(px, py);
            }
        }
        else if (line.rfind("PixelsPerUnit: ", 0) == 0)
        {
            std::stringstream ss(line.substr(15));
            float ppu;
            if (ss >> ppu)
            {
                m_Sprite->SetPixelsPerUnit(ppu);
            }
        }
        else if (line.rfind("ColliderPoints: ", 0) == 0)
        {
            std::stringstream ss(line.substr(16));
            size_t count = 0;
            if (ss >> count)
            {
                std::vector<TEVector2> pts;
                for (size_t i = 0; i < count; ++i)
                {
                    float x, y;
                    if (ss >> x >> y)
                    {
                        pts.push_back({ x, y });
                    }
                }
                m_Sprite->SetCustomColliderPoints(pts);
            }
        }
    }

    hin.close();
    return true;
}

} // namespace TE
