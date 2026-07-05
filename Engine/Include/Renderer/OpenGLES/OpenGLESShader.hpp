#pragma once
#include "Renderer/Shader.hpp"

namespace TE {

class OpenGLESShader : public Shader {
public:
    OpenGLESShader(const std::string& vertexSrc, const std::string& fragmentSrc);
    virtual ~OpenGLESShader();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void SetUniformMat4(const std::string& name, const glm::mat4& value) override;
    virtual void SetUniform4f(const std::string& name, const glm::vec4& value) override;
    virtual void SetUniform3f(const std::string& name, const glm::vec3& value) override;
    virtual void SetUniform2f(const std::string& name, const glm::vec2& value) override;
    virtual void SetUniform1f(const std::string& name, float value) override;
    virtual void SetUniform1i(const std::string& name, int value) override;

    // ===== OpenGL ES-Specific Methods =====
    uint32_t GetRendererID() const { return m_RendererID; }
    int GetUniformLocation(const std::string& name);

private:
    uint32_t m_RendererID;
};

} // namespace TE
