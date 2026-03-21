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

void Material::SetUniform(const std::string& name, float value) { m_FloatUniforms[name] = value; }
void Material::SetUniform(const std::string& name, int value) { m_IntUniforms[name] = value; }
void Material::SetUniform(const std::string& name, const glm::vec2& value) { m_Vec2Uniforms[name] = value; }
void Material::SetUniform(const std::string& name, const glm::vec3& value) { m_Vec3Uniforms[name] = value; }
void Material::SetUniform(const std::string& name, const glm::vec4& value) { m_Vec4Uniforms[name] = value; }
void Material::SetUniform(const std::string& name, const glm::mat4& value) { m_Mat4Uniforms[name] = value; }

void Material::ApplyUniforms() {
    if (m_Shader) {
        ShaderLibrary::SetColor(m_Shader.get(), m_Color);
        
        for (auto const& [name, val] : m_FloatUniforms) m_Shader->SetUniform1f(name, val);
        for (auto const& [name, val] : m_IntUniforms) m_Shader->SetUniform1i(name, val);
        for (auto const& [name, val] : m_Vec2Uniforms) m_Shader->SetUniform2f(name, val);
        for (auto const& [name, val] : m_Vec3Uniforms) m_Shader->SetUniform3f(name, val);
        for (auto const& [name, val] : m_Vec4Uniforms) m_Shader->SetUniform4f(name, val);
        for (auto const& [name, val] : m_Mat4Uniforms) m_Shader->SetUniformMat4(name, val);
    }
}

} // namespace TE 