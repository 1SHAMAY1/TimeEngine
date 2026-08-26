#include "Core/PreRequisites.h"
#include "Renderer/MaterialInstance.hpp"
#include "Renderer/ShaderLibrary.hpp"

MaterialInstance::MaterialInstance(const TERef<Material> &baseMaterial) : m_BaseMaterial(baseMaterial) {}

MaterialInstance::~MaterialInstance() {}

void MaterialInstance::SetColor(const TEColor &color)
{
    m_OverrideColor = color;
    m_HasOverrideColor = true;
}

const TEColor &MaterialInstance::GetColor() const
{
    if (m_HasOverrideColor)
        return m_OverrideColor;
    return m_BaseMaterial->GetColor();
}

TERef<Material> MaterialInstance::GetBaseMaterial() const { return m_BaseMaterial; }

void MaterialInstance::ApplyUniforms()
{
    if (m_BaseMaterial)
    {
        auto shader = m_BaseMaterial->GetShader();
        if (shader)
        {
            ShaderLibrary::SetColor(shader.get(), GetColor());
        }
    }
}
