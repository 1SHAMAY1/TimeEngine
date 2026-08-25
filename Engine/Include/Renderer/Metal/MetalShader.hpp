#pragma once

#include "Renderer/Shader.hpp"

class MetalShader : public Shader
{
public:
    MetalShader(const TEString &vertexSrc, const TEString &fragmentSrc);
    MetalShader(const TEString &computeSrc);
    virtual ~MetalShader();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void SetUniformMat4(const TEString &name, const glm::mat4 &value) override;
    virtual void SetUniform4f(const TEString &name, const glm::vec4 &value) override;
    virtual void SetUniform3f(const TEString &name, const glm::vec3 &value) override;
    virtual void SetUniform2f(const TEString &name, const glm::vec2 &value) override;
    virtual void SetUniform1f(const TEString &name, float value) override;
    virtual void SetUniform1i(const TEString &name, int value) override;

    void *GetLibrary() const { return m_Library; }

private:
    void *m_Library = nullptr;
    void *m_PipelineState = nullptr;
};

