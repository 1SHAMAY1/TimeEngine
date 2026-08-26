#include "Renderer/MaterialLibrary.hpp"
#include "Core/PreRequisites.h"

TEMap<TEString, TERef<Material>> MaterialLibrary::s_Materials;

void MaterialLibrary::Register(const TEString &name, const TERef<Material> &material) { s_Materials[name] = material; }

TERef<Material> MaterialLibrary::Get(const TEString &name)
{
    auto it = s_Materials.find(name);
    if (it != s_Materials.end())
        return it->second;
    return nullptr;
}

bool MaterialLibrary::Exists(const TEString &name) { return s_Materials.find(name) != s_Materials.end(); }
