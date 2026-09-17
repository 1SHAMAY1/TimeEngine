#pragma once
#include "PreRequisites.h"
#include "Material.hpp"

class TE_API MaterialLibrary
{
public:
    static void Register(const TEString &name, const TERef<Material> &material);
    static TERef<Material> Get(const TEString &name);
    static bool Exists(const TEString &name);

private:
    static TEMap<TEString, TERef<Material>> s_Materials;
};
