#include "Renderer/MaterialSerializer.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE
{

MaterialSerializer::MaterialSerializer(const std::shared_ptr<Material> &material) : m_Material(material) {}

bool MaterialSerializer::Serialize(const std::filesystem::path &filepath)
{
    std::ofstream hout(filepath);
    if (!hout.is_open())
    {
        TE_CORE_ERROR("MaterialSerializer: Failed to open file for writing at {0}", filepath.string());
        return false;
    }

    hout << "Material: " << m_Material->GetName() << "\n";

    auto color = m_Material->GetColor().GetValue();
    hout << "Color: " << color.r << " " << color.g << " " << color.b << " " << color.a << "\n";

    hout.close();
    return true;
}

bool MaterialSerializer::Deserialize(const std::filesystem::path &filepath)
{
    std::ifstream hin(filepath);
    if (!hin.is_open())
        return false;

    std::string line;
    while (std::getline(hin, line))
    {
        if (line.find("Material: ") == 0)
        {
            m_Material->SetName(line.substr(10));
        }
        else if (line.find("Color: ") == 0)
        {
            std::stringstream ss(line.substr(7));
            float r, g, b, a;
            ss >> r >> g >> b >> a;
            m_Material->SetColor(TEColor(r, g, b, a));
        }
    }

    hin.close();
    return true;
}

} // namespace TE
