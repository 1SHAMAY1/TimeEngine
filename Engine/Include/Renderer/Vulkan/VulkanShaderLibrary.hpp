#pragma once
#include "Renderer/Vulkan/VulkanShader.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include <volk.h>
#include <vector>
#include <string>
#include <memory>

namespace TE
{

class VulkanShaderLibrary : public ShaderLibrary
{
public:
    // ===== Vulkan-Specific Shader Creation =====
    static std::shared_ptr<VulkanShader> CreateVulkanBasicShader();
    static std::shared_ptr<VulkanShader> CreateVulkanTextureShader();
    static std::shared_ptr<VulkanShader> CreateVulkanColorShader();
    static std::shared_ptr<VulkanShader> CreateVulkanStandardShader();
    static std::shared_ptr<VulkanShader> CreateVulkanLightingShader();
    static std::shared_ptr<VulkanShader> CreateVulkanParticleShader();
    static std::shared_ptr<VulkanShader> CreateVulkanPostProcessShader();
    static std::shared_ptr<VulkanShader> CreateVulkanUIShader();
    static std::shared_ptr<VulkanShader> CreateVulkanLight2DShader();
    static std::shared_ptr<VulkanShader> CreateVulkanComputeShader(const std::string &computeSource);

    // ===== Vulkan-Specific Functions =====
    static void SetUniform1i(VulkanShader *shader, const std::string &name, int value);
    static void SetUniform1f(VulkanShader *shader, const std::string &name, float value);
    static void SetUniform2f(VulkanShader *shader, const std::string &name, const glm::vec2 &value);
    static void SetUniform3f(VulkanShader *shader, const std::string &name, const glm::vec3 &value);
    static void SetUniform4f(VulkanShader *shader, const std::string &name, const glm::vec4 &value);
    static void SetUniformMat3(VulkanShader *shader, const std::string &name, const glm::mat3 &value);
    static void SetUniformMat4(VulkanShader *shader, const std::string &name, const glm::mat4 &value);
    static void SetUniformMat4Array(VulkanShader *shader, const std::string &name,
                                    const std::vector<glm::mat4> &values);

    // ===== Vulkan State Management =====
    static void EnableBlending();
    static void DisableBlending();
    static void SetBlendFunc(uint32_t srcFactor, uint32_t dstFactor);
    static void EnableDepthTest();
    static void DisableDepthTest();
    static void SetDepthFunc(uint32_t func);
    static void EnableStencilTest();
    static void DisableStencilTest();
    static void SetStencilFunc(uint32_t func, int ref, unsigned int mask);
    static void SetStencilOp(uint32_t sfail, uint32_t dpfail, uint32_t dppass);
    static void EnableCullFace();
    static void DisableCullFace();
    static void SetCullFace(uint32_t face);
    static void SetFrontFace(uint32_t mode);

    // ===== Vulkan Texture Functions =====
    static void BindTexture2D(unsigned int textureID, int slot);
    static void BindTextureCube(unsigned int textureID, int slot);
    static void SetTextureFiltering(unsigned int textureID, uint32_t minFilter, uint32_t magFilter);
    static void SetTextureWrapping(unsigned int textureID, uint32_t wrapS, uint32_t wrapT);
    static void GenerateMipmaps(unsigned int textureID);

    // ===== Vulkan Framebuffer Functions =====
    static unsigned int CreateFramebuffer();
    static void BindFramebuffer(unsigned int framebufferID);
    static void AttachTexture2D(unsigned int framebufferID, unsigned int textureID, uint32_t attachment);
    static void AttachRenderbuffer(unsigned int framebufferID, unsigned int renderbufferID, uint32_t attachment);
    static void SetDrawBuffers(const std::vector<uint32_t> &attachments);
    static bool CheckFramebufferStatus(unsigned int framebufferID);
    static void DeleteFramebuffer(unsigned int framebufferID);

    // ===== Vulkan Compute Shader Functions =====
    static void DispatchCompute(unsigned int shaderID, unsigned int numGroupsX, unsigned int numGroupsY,
                                unsigned int numGroupsZ);
    static void MemoryBarrier(uint32_t barriers);
    static void BindImageTexture(unsigned int unit, unsigned int textureID, int level, bool layered, int layer,
                                 uint32_t access, uint32_t format);

    // ===== Vulkan Uniform Buffer Functions =====
    static unsigned int CreateUniformBuffer(const void *data, size_t size);
    static void BindUniformBuffer(unsigned int uboID, unsigned int bindingPoint);
    static void UpdateUniformBuffer(unsigned int uboID, const void *data, size_t offset, size_t size);
    static void BindUniformBlock(VulkanShader *shader, const std::string &blockName, unsigned int bindingPoint);

    // ===== Vulkan Vertex Array Functions =====
    static void SetVertexAttribPointer(unsigned int index, int size, uint32_t type, bool normalized, int stride,
                                       const void *pointer);
    static void EnableVertexAttribArray(unsigned int index);
    static void DisableVertexAttribArray(unsigned int index);
    static void VertexAttribDivisor(unsigned int index, unsigned int divisor);

    // ===== Vulkan Debug Functions =====
    static void EnableDebugOutput();
    static void SetDebugCallback();
    static void PushDebugGroup(const std::string &message);
    static void PopDebugGroup();
    static void ObjectLabel(uint32_t identifier, unsigned int name, const std::string &label);

    // ===== Vulkan Performance Functions =====
    static void BeginQuery(uint32_t target, unsigned int id);
    static void EndQuery(uint32_t target);
    static int GetQueryObjecti(unsigned int id, uint32_t pname);
    static void GetQueryObjectiv(unsigned int id, uint32_t pname, int *params);
    static void GetQueryObjectuiv(unsigned int id, uint32_t pname, unsigned int *params);

    // ===== Vulkan Synchronization =====
    static void FenceSync(uint32_t condition, uint32_t flags);
    static void ClientWaitSync(void *sync, uint32_t flags, uint64_t timeout);
    static void DeleteSync(void *sync);
};

} // namespace TE
