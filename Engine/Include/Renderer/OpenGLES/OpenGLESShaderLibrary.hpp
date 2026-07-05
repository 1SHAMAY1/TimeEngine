#pragma once
#include "Renderer/OpenGLES/OpenGLESShader.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include <glad/glad.h>
#include <vector>
#include <string>
#include <memory>

namespace TE {

class OpenGLESShaderLibrary : public ShaderLibrary {
public:
    // ===== OpenGL ES-Specific Shader Creation =====
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESBasicShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESTextureShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESColorShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESStandardShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESLightingShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESParticleShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESPostProcessShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESUIShader();
    static std::shared_ptr<OpenGLESShader> CreateOpenGLESLight2DShader();

    // ===== OpenGL ES-Specific Uniform Functions =====
    static void SetUniform1i(OpenGLESShader* shader, const std::string& name, int value);
    static void SetUniform1f(OpenGLESShader* shader, const std::string& name, float value);
    static void SetUniform2f(OpenGLESShader* shader, const std::string& name, const glm::vec2& value);
    static void SetUniform3f(OpenGLESShader* shader, const std::string& name, const glm::vec3& value);
    static void SetUniform4f(OpenGLESShader* shader, const std::string& name, const glm::vec4& value);
    static void SetUniformMat3(OpenGLESShader* shader, const std::string& name, const glm::mat3& value);
    static void SetUniformMat4(OpenGLESShader* shader, const std::string& name, const glm::mat4& value);
    static void SetUniformMat4Array(OpenGLESShader* shader, const std::string& name,
                                    const std::vector<glm::mat4>& values);

    // ===== OpenGL ES State Management =====
    static void EnableBlending();
    static void DisableBlending();
    static void SetBlendFunc(GLenum srcFactor, GLenum dstFactor);
    static void EnableDepthTest();
    static void DisableDepthTest();
    static void SetDepthFunc(GLenum func);
    static void EnableStencilTest();
    static void DisableStencilTest();
    static void SetStencilFunc(GLenum func, int ref, unsigned int mask);
    static void SetStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
    static void EnableCullFace();
    static void DisableCullFace();
    static void SetCullFace(GLenum face);
    static void SetFrontFace(GLenum mode);

    // ===== OpenGL ES Texture Functions =====
    static void BindTexture2D(unsigned int textureID, int slot);
    static void BindTextureCube(unsigned int textureID, int slot);
    static void SetTextureFiltering(unsigned int textureID, GLenum minFilter, GLenum magFilter);
    static void SetTextureWrapping(unsigned int textureID, GLenum wrapS, GLenum wrapT);
    static void GenerateMipmaps(unsigned int textureID);

    // ===== OpenGL ES Framebuffer Functions =====
    static unsigned int CreateFramebuffer();
    static void BindFramebuffer(unsigned int framebufferID);
    static void AttachTexture2D(unsigned int framebufferID, unsigned int textureID, GLenum attachment);
    static void AttachRenderbuffer(unsigned int framebufferID, unsigned int renderbufferID, GLenum attachment);
    static bool CheckFramebufferStatus(unsigned int framebufferID);
    static void DeleteFramebuffer(unsigned int framebufferID);

    // ===== OpenGL ES Uniform Buffer Functions (ES 3.0+) =====
    static unsigned int CreateUniformBuffer(const void* data, size_t size);
    static void BindUniformBuffer(unsigned int uboID, unsigned int bindingPoint);
    static void UpdateUniformBuffer(unsigned int uboID, const void* data, size_t offset, size_t size);
    static void BindUniformBlock(OpenGLESShader* shader, const std::string& blockName, unsigned int bindingPoint);

    // ===== OpenGL ES Vertex Array Functions =====
    static void SetVertexAttribPointer(unsigned int index, int size, GLenum type, bool normalized, int stride,
                                       const void* pointer);
    static void EnableVertexAttribArray(unsigned int index);
    static void DisableVertexAttribArray(unsigned int index);
    static void VertexAttribDivisor(unsigned int index, unsigned int divisor);

    // ===== OpenGL ES Debug Functions (via KHR_debug extension) =====
    static void EnableDebugOutput();
    static void SetDebugCallback();
    static void PushDebugGroup(const std::string& message);
    static void PopDebugGroup();
    static void ObjectLabel(GLenum identifier, unsigned int name, const std::string& label);

    // ===== OpenGL ES Query Functions =====
    static void BeginQuery(GLenum target, unsigned int id);
    static void EndQuery(GLenum target);
    static int GetQueryObjecti(unsigned int id, GLenum pname);
    static void GetQueryObjectiv(unsigned int id, GLenum pname, int* params);
    static void GetQueryObjectuiv(unsigned int id, GLenum pname, unsigned int* params);

    // ===== OpenGL ES Sync / Fence (ES 3.0+) =====
    static GLsync FenceSync(GLenum condition, GLbitfield flags);
    static GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
    static void   DeleteSync(GLsync sync);

private:
    static void GLAPIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                         GLsizei length, const char* message, const void* userParam);
};

} // namespace TE
