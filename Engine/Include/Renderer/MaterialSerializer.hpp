#pragma once
#include "Renderer/Material.hpp"
class MaterialSerializer
{
public:
    MaterialSerializer(const TERef<Material> &material);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath);

private:
    TERef<Material> m_Material;
};

