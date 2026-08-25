#pragma once
#include "Renderer/Material.hpp"
#include <memory>

class TE_API MaterialInstance {
public:
    MaterialInstance(const TERef<Material>& baseMaterial);
    ~MaterialInstance();

    void SetColor(const TEColor& color);
    const TEColor& GetColor() const;

    TERef<Material> GetBaseMaterial() const;

    void ApplyUniforms();

private:
    TERef<Material> m_BaseMaterial;
    TEColor m_OverrideColor;
    bool m_HasOverrideColor = false;
};

