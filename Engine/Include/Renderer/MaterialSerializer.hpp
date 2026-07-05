#pragma once
#include "Renderer/Material.hpp"
#include <filesystem>
#include <memory>

namespace TE
{

class MaterialSerializer
{
public:
    MaterialSerializer(const std::shared_ptr<Material> &material);

    bool Serialize(const std::filesystem::path &filepath);
    bool Deserialize(const std::filesystem::path &filepath);

private:
    std::shared_ptr<Material> m_Material;
};

} // namespace TE
