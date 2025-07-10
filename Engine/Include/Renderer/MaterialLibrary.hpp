#pragma once
#include "Renderer/Material.hpp"
#include <unordered_map>
#include <memory>
#include <string>

namespace TE {

class MaterialLibrary {
public:
    static void Register(const std::string& name, const std::shared_ptr<Material>& material);
    static std::shared_ptr<Material> Get(const std::string& name);
    static bool Exists(const std::string& name);

private:
    static std::unordered_map<std::string, std::shared_ptr<Material>> s_Materials;
};

} 