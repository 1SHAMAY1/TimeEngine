#pragma once

#include "Renderer/Shader.hpp"

namespace TE
{
class MetalShader : public Shader
{
public:
    MetalShader(const std::string &vertexSrc, const std::string &fragmentSrc);
    MetalShader(const std::string &computeSrc);
    virtual ~MetalShader();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void SetUniformMat4(const std::string &name, const glm::mat4 &value) override;
    virtual void SetUniform4f(const std::string &name, const glm::vec4 &value) override;
    virtual void SetUniform3f(const std::string &name, const glm::vec3 &value) override;
    virtual void SetUniform2f(const std::string &name, const glm::vec2 &value) override;
    virtual void SetUniform1f(const std::string &name, float value) override;
    virtual void SetUniform1i(const std::string &name, int value) override;

    void *GetLibrary() const { return m_Library; }

private:
    void *m_Library = nullptr;
    void *m_PipelineState = nullptr;
};
} // namespace TE
