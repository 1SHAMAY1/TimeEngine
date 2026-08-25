#pragma once
#include "Renderer/Shader.hpp"


class OpenGLESShader : public Shader
{
public:
    OpenGLESShader(const TEString &vertexSrc, const TEString &fragmentSrc);
    virtual ~OpenGLESShader();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void SetUniformMat4(const TEString &name, const glm::mat4 &value) override;
    virtual void SetUniform4f(const TEString &name, const glm::vec4 &value) override;
    virtual void SetUniform3f(const TEString &name, const glm::vec3 &value) override;
    virtual void SetUniform2f(const TEString &name, const glm::vec2 &value) override;
    virtual void SetUniform1f(const TEString &name, float value) override;
    virtual void SetUniform1i(const TEString &name, int value) override;

    // ===== OpenGL ES-Specific Methods =====
    uint32_t GetRendererID() const { return m_RendererID; }
    int GetUniformLocation(const TEString &name);

private:
    uint32_t m_RendererID;
};

