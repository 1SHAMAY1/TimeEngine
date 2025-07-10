#include "Renderer/Material.hpp"
#include "Renderer/ShaderLibrary.hpp"

namespace TE {

Material::Material(const std::shared_ptr<Shader>& shader)
    : m_Shader(shader), m_Color(TEColor::White()) {}

Material::~Material() {}

void Material::SetColor(const TEColor& color) {
    m_Color = color;
}

const TEColor& Material::GetColor() const {
    return m_Color;
}

void Material::SetShader(const std::shared_ptr<Shader>& shader) {
    m_Shader = shader;
}

std::shared_ptr<Shader> Material::GetShader() const {
    return m_Shader;
}

void Material::ApplyUniforms() {
    if (m_Shader) {
        ShaderLibrary::SetColor(m_Shader.get(), m_Color);
    }
}

} // namespace TE 