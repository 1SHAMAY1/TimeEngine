#include "Renderer/MaterialLibrary.hpp"

namespace TE {

std::unordered_map<std::string, std::shared_ptr<Material>> MaterialLibrary::s_Materials;

void MaterialLibrary::Register(const std::string& name, const std::shared_ptr<Material>& material) {
    s_Materials[name] = material;
}

std::shared_ptr<Material> MaterialLibrary::Get(const std::string& name) {
    auto it = s_Materials.find(name);
    if (it != s_Materials.end())
        return it->second;
    return nullptr;
}

bool MaterialLibrary::Exists(const std::string& name) {
    return s_Materials.find(name) != s_Materials.end();
}

} // namespace TE 