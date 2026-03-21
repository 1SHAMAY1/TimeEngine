#pragma once
#include "Renderer/Shader.hpp"
#include "Renderer/TEColor.hpp"
#include <glm/glm.hpp>
#include <unordered_map>

namespace TE
{

class TE_API Material
{
public:
    Material(const std::shared_ptr<Shader> &shader);
    ~Material();

    void SetColor(const TEColor &color);
    const TEColor &GetColor() const;

    void SetUniform(const std::string &name, float value);
    void SetUniform(const std::string &name, int value);
    void SetUniform(const std::string &name, const glm::vec2 &value);
    void SetUniform(const std::string &name, const glm::vec3 &value);
    void SetUniform(const std::string &name, const glm::vec4 &value);
    void SetUniform(const std::string &name, const glm::mat4 &value);

    void SetShader(const std::shared_ptr<Shader> &shader);
    std::shared_ptr<Shader> GetShader() const;

    // Set all uniforms (for now, just color and custom ones)
    void ApplyUniforms();

private:
    std::shared_ptr<Shader> m_Shader;
    TEColor m_Color;

    std::unordered_map<std::string, float> m_FloatUniforms;
    std::unordered_map<std::string, int> m_IntUniforms;
    std::unordered_map<std::string, glm::vec2> m_Vec2Uniforms;
    std::unordered_map<std::string, glm::vec3> m_Vec3Uniforms;
    std::unordered_map<std::string, glm::vec4> m_Vec4Uniforms;
    std::unordered_map<std::string, glm::mat4> m_Mat4Uniforms;
};

} // namespace TE