#pragma once
#include "Renderer/Shader.hpp"
#include <unordered_map>
#include <volk.h>

class VulkanShader : public Shader
{
public:
    VulkanShader(const TEString &vertexSrc, const TEString &fragmentSrc);
    VulkanShader(const TEString &computeSrc);
    virtual ~VulkanShader() override;

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void SetUniformMat4(const TEString &name, const glm::mat4 &value) override;
    virtual void SetUniform4f(const TEString &name, const glm::vec4 &value) override;
    virtual void SetUniform3f(const TEString &name, const glm::vec3 &value) override;
    virtual void SetUniform2f(const TEString &name, const glm::vec2 &value) override;
    virtual void SetUniform1f(const TEString &name, float value) override;
    virtual void SetUniform1i(const TEString &name, int value) override;

    VkShaderModule GetVertexShaderModule() const { return m_VertexModule; }
    VkShaderModule GetFragmentShaderModule() const { return m_FragmentModule; }
    VkShaderModule GetComputeShaderModule() const { return m_ComputeModule; }

private:
    VkShaderModule m_VertexModule = VK_NULL_HANDLE;
    VkShaderModule m_FragmentModule = VK_NULL_HANDLE;
    VkShaderModule m_ComputeModule = VK_NULL_HANDLE;
};

