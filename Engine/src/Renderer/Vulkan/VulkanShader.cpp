#include "Renderer/Vulkan/VulkanShader.hpp"
#include "Renderer/Vulkan/VulkanContext.hpp"
#include <iostream>

namespace TE
{
    VulkanShader::VulkanShader(const std::string &vertexSrc, const std::string &fragmentSrc)
    {
        VkDevice device = VulkanContext::GetDevice();
        if (device == VK_NULL_HANDLE) {
            std::cout << "[Vulkan] Error: Device is null, skipping Shader modules creation\n";
            return;
        }

        // Vertex shader module creation
        VkShaderModuleCreateInfo vertCreateInfo{};
        vertCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertCreateInfo.codeSize = vertexSrc.size();
        vertCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexSrc.data());

        if (vkCreateShaderModule(device, &vertCreateInfo, nullptr, &m_VertexModule) != VK_SUCCESS) {
            std::cout << "[Vulkan] Error: Failed to create vertex shader module\n";
        }

        // Fragment shader module creation
        VkShaderModuleCreateInfo fragCreateInfo{};
        fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragCreateInfo.codeSize = fragmentSrc.size();
        fragCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentSrc.data());

        if (vkCreateShaderModule(device, &fragCreateInfo, nullptr, &m_FragmentModule) != VK_SUCCESS) {
            std::cout << "[Vulkan] Error: Failed to create fragment shader module\n";
        }
    }

    VulkanShader::VulkanShader(const std::string &computeSrc)
    {
        VkDevice device = VulkanContext::GetDevice();
        if (device == VK_NULL_HANDLE) {
            std::cout << "[Vulkan] Error: Device is null, skipping Compute Shader module creation\n";
            return;
        }

        VkShaderModuleCreateInfo compCreateInfo{};
        compCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        compCreateInfo.codeSize = computeSrc.size();
        compCreateInfo.pCode = reinterpret_cast<const uint32_t*>(computeSrc.data());

        if (vkCreateShaderModule(device, &compCreateInfo, nullptr, &m_ComputeModule) != VK_SUCCESS) {
            std::cout << "[Vulkan] Error: Failed to create compute shader module\n";
        }
    }

    VulkanShader::~VulkanShader()
    {
        VkDevice device = VulkanContext::GetDevice();
        if (device != VK_NULL_HANDLE) {
            if (m_VertexModule != VK_NULL_HANDLE) {
                vkDestroyShaderModule(device, m_VertexModule, nullptr);
            }
            if (m_FragmentModule != VK_NULL_HANDLE) {
                vkDestroyShaderModule(device, m_FragmentModule, nullptr);
            }
            if (m_ComputeModule != VK_NULL_HANDLE) {
                vkDestroyShaderModule(device, m_ComputeModule, nullptr);
            }
        }
    }

    void VulkanShader::Bind() const
    {
        // Pipeline binding logic
    }

    void VulkanShader::Unbind() const
    {
    }

    void VulkanShader::SetUniformMat4(const std::string &name, const glm::mat4 &value)
    {
    }

    void VulkanShader::SetUniform4f(const std::string &name, const glm::vec4 &value)
    {
    }

    void VulkanShader::SetUniform3f(const std::string &name, const glm::vec3 &value)
    {
    }

    void VulkanShader::SetUniform2f(const std::string &name, const glm::vec2 &value)
    {
    }

    void VulkanShader::SetUniform1f(const std::string &name, float value)
    {
    }

    void VulkanShader::SetUniform1i(const std::string &name, int value)
    {
    }

} // namespace TE
