#pragma once
#include "Renderer/Material.hpp"
#include <memory>

namespace TE {

class MaterialInstance {
public:
    MaterialInstance(const std::shared_ptr<Material>& baseMaterial);
    ~MaterialInstance();

    void SetColor(const TEColor& color);
    const TEColor& GetColor() const;

    std::shared_ptr<Material> GetBaseMaterial() const;

    void ApplyUniforms();

private:
    std::shared_ptr<Material> m_BaseMaterial;
    TEColor m_OverrideColor;
    bool m_HasOverrideColor = false;
};

} 