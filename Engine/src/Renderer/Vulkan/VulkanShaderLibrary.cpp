#include "Renderer/Vulkan/VulkanShaderLibrary.hpp"
#include <iostream>

namespace TE
{

    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanBasicShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanTextureShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanColorShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanStandardShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanLightingShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanParticleShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanPostProcessShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanUIShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanLight2DShader() { return nullptr; }
    std::shared_ptr<VulkanShader> VulkanShaderLibrary::CreateVulkanComputeShader(const std::string &computeSource) { return nullptr; }

    void VulkanShaderLibrary::SetUniform1i(VulkanShader *shader, const std::string &name, int value) {}
    void VulkanShaderLibrary::SetUniform1f(VulkanShader *shader, const std::string &name, float value) {}
    void VulkanShaderLibrary::SetUniform2f(VulkanShader *shader, const std::string &name, const glm::vec2 &value) {}
    void VulkanShaderLibrary::SetUniform3f(VulkanShader *shader, const std::string &name, const glm::vec3 &value) {}
    void VulkanShaderLibrary::SetUniform4f(VulkanShader *shader, const std::string &name, const glm::vec4 &value) {}
    void VulkanShaderLibrary::SetUniformMat3(VulkanShader *shader, const std::string &name, const glm::mat3 &value) {}
    void VulkanShaderLibrary::SetUniformMat4(VulkanShader *shader, const std::string &name, const glm::mat4 &value) {}
    void VulkanShaderLibrary::SetUniformMat4Array(VulkanShader *shader, const std::string &name, const std::vector<glm::mat4> &values) {}

    void VulkanShaderLibrary::EnableBlending() {}
    void VulkanShaderLibrary::DisableBlending() {}
    void VulkanShaderLibrary::SetBlendFunc(uint32_t srcFactor, uint32_t dstFactor) {}
    void VulkanShaderLibrary::EnableDepthTest() {}
    void VulkanShaderLibrary::DisableDepthTest() {}
    void VulkanShaderLibrary::SetDepthFunc(uint32_t func) {}
    void VulkanShaderLibrary::EnableStencilTest() {}
    void VulkanShaderLibrary::DisableStencilTest() {}
    void VulkanShaderLibrary::SetStencilFunc(uint32_t func, int ref, unsigned int mask) {}
    void VulkanShaderLibrary::SetStencilOp(uint32_t sfail, uint32_t dpfail, uint32_t dppass) {}
    void VulkanShaderLibrary::EnableCullFace() {}
    void VulkanShaderLibrary::DisableCullFace() {}
    void VulkanShaderLibrary::SetCullFace(uint32_t face) {}
    void VulkanShaderLibrary::SetFrontFace(uint32_t mode) {}

    void VulkanShaderLibrary::BindTexture2D(unsigned int textureID, int slot) {}
    void VulkanShaderLibrary::BindTextureCube(unsigned int textureID, int slot) {}
    void VulkanShaderLibrary::SetTextureFiltering(unsigned int textureID, uint32_t minFilter, uint32_t magFilter) {}
    void VulkanShaderLibrary::SetTextureWrapping(unsigned int textureID, uint32_t wrapS, uint32_t wrapT) {}
    void VulkanShaderLibrary::GenerateMipmaps(unsigned int textureID) {}

    unsigned int VulkanShaderLibrary::CreateFramebuffer() { return 0; }
    void VulkanShaderLibrary::BindFramebuffer(unsigned int framebufferID) {}
    void VulkanShaderLibrary::AttachTexture2D(unsigned int framebufferID, unsigned int textureID, uint32_t attachment) {}
    void VulkanShaderLibrary::AttachRenderbuffer(unsigned int framebufferID, unsigned int renderbufferID, uint32_t attachment) {}
    void VulkanShaderLibrary::SetDrawBuffers(const std::vector<uint32_t> &attachments) {}
    bool VulkanShaderLibrary::CheckFramebufferStatus(unsigned int framebufferID) { return true; }
    void VulkanShaderLibrary::DeleteFramebuffer(unsigned int framebufferID) {}

    void VulkanShaderLibrary::DispatchCompute(unsigned int shaderID, unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) {}
    void VulkanShaderLibrary::MemoryBarrier(uint32_t barriers) {}
    void VulkanShaderLibrary::BindImageTexture(unsigned int unit, unsigned int textureID, int level, bool layered, int layer, uint32_t access, uint32_t format) {}

    unsigned int VulkanShaderLibrary::CreateUniformBuffer(const void *data, size_t size) { return 0; }
    void VulkanShaderLibrary::BindUniformBuffer(unsigned int uboID, unsigned int bindingPoint) {}
    void VulkanShaderLibrary::UpdateUniformBuffer(unsigned int uboID, const void *data, size_t offset, size_t size) {}
    void VulkanShaderLibrary::BindUniformBlock(VulkanShader *shader, const std::string &blockName, unsigned int bindingPoint) {}

    void VulkanShaderLibrary::SetVertexAttribPointer(unsigned int index, int size, uint32_t type, bool normalized, int stride, const void *pointer) {}
    void VulkanShaderLibrary::EnableVertexAttribArray(unsigned int index) {}
    void VulkanShaderLibrary::DisableVertexAttribArray(unsigned int index) {}
    void VulkanShaderLibrary::VertexAttribDivisor(unsigned int index, unsigned int divisor) {}

    void VulkanShaderLibrary::EnableDebugOutput() {}
    void VulkanShaderLibrary::SetDebugCallback() {}
    void VulkanShaderLibrary::PushDebugGroup(const std::string &message) {}
    void VulkanShaderLibrary::PopDebugGroup() {}
    void VulkanShaderLibrary::ObjectLabel(uint32_t identifier, unsigned int name, const std::string &label) {}

    void VulkanShaderLibrary::BeginQuery(uint32_t target, unsigned int id) {}
    void VulkanShaderLibrary::EndQuery(uint32_t target) {}
    int VulkanShaderLibrary::GetQueryObjecti(unsigned int id, uint32_t pname) { return 0; }
    void VulkanShaderLibrary::GetQueryObjectiv(unsigned int id, uint32_t pname, int *params) {}
    void VulkanShaderLibrary::GetQueryObjectuiv(unsigned int id, uint32_t pname, unsigned int *params) {}

    void VulkanShaderLibrary::FenceSync(uint32_t condition, uint32_t flags) {}
    void VulkanShaderLibrary::ClientWaitSync(void *sync, uint32_t flags, uint64_t timeout) {}
    void VulkanShaderLibrary::DeleteSync(void *sync) {}

} // namespace TE
