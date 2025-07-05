#pragma once
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/OpenGL/OpenGLShader.hpp"
#include <glad/glad.h>

namespace TE {

    class OpenGLShaderLibrary : public ShaderLibrary {
    public:
        // ===== OpenGL-Specific Shader Creation =====
        static std::shared_ptr<OpenGLShader> CreateOpenGLBasicShader();
        static std::shared_ptr<OpenGLShader> CreateOpenGLTextureShader();
        static std::shared_ptr<OpenGLShader> CreateOpenGLColorShader();
        static std::shared_ptr<OpenGLShader> CreateOpenGLLightingShader();
        static std::shared_ptr<OpenGLShader> CreateOpenGLParticleShader();
        static std::shared_ptr<OpenGLShader> CreateOpenGLPostProcessShader();
        static std::shared_ptr<OpenGLShader> CreateOpenGLUIShader();
        static std::shared_ptr<OpenGLShader> CreateOpenGLComputeShader(const std::string& computeSource);

        // ===== OpenGL-Specific Functions =====
        static void SetUniform1i(OpenGLShader* shader, const std::string& name, int value);
        static void SetUniform1f(OpenGLShader* shader, const std::string& name, float value);
        static void SetUniform2f(OpenGLShader* shader, const std::string& name, const glm::vec2& value);
        static void SetUniform3f(OpenGLShader* shader, const std::string& name, const glm::vec3& value);
        static void SetUniform4f(OpenGLShader* shader, const std::string& name, const glm::vec4& value);
        static void SetUniformMat3(OpenGLShader* shader, const std::string& name, const glm::mat3& value);
        static void SetUniformMat4(OpenGLShader* shader, const std::string& name, const glm::mat4& value);
        static void SetUniformMat4Array(OpenGLShader* shader, const std::string& name, const std::vector<glm::mat4>& values);

        // ===== OpenGL State Management =====
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

        // ===== OpenGL Texture Functions =====
        static void BindTexture2D(unsigned int textureID, int slot);
        static void BindTextureCube(unsigned int textureID, int slot);
        static void SetTextureFiltering(unsigned int textureID, GLenum minFilter, GLenum magFilter);
        static void SetTextureWrapping(unsigned int textureID, GLenum wrapS, GLenum wrapT);
        static void GenerateMipmaps(unsigned int textureID);

        // ===== OpenGL Framebuffer Functions =====
        static unsigned int CreateFramebuffer();
        static void BindFramebuffer(unsigned int framebufferID);
        static void AttachTexture2D(unsigned int framebufferID, unsigned int textureID, GLenum attachment);
        static void AttachRenderbuffer(unsigned int framebufferID, unsigned int renderbufferID, GLenum attachment);
        static void SetDrawBuffers(const std::vector<GLenum>& attachments);
        static bool CheckFramebufferStatus(unsigned int framebufferID);
        static void DeleteFramebuffer(unsigned int framebufferID);

        // ===== OpenGL Compute Shader Functions =====
        static void DispatchCompute(unsigned int shaderID, unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ);
        static void MemoryBarrier(GLenum barriers);
        static void BindImageTexture(unsigned int unit, unsigned int textureID, int level, bool layered, int layer, GLenum access, GLenum format);

        // ===== OpenGL Uniform Buffer Functions =====
        static unsigned int CreateUniformBuffer(const void* data, size_t size);
        static void BindUniformBuffer(unsigned int uboID, unsigned int bindingPoint);
        static void UpdateUniformBuffer(unsigned int uboID, const void* data, size_t offset, size_t size);
        static void BindUniformBlock(OpenGLShader* shader, const std::string& blockName, unsigned int bindingPoint);

        // ===== OpenGL Vertex Array Functions =====
        static void SetVertexAttribPointer(unsigned int index, int size, GLenum type, bool normalized, int stride, const void* pointer);
        static void EnableVertexAttribArray(unsigned int index);
        static void DisableVertexAttribArray(unsigned int index);
        static void VertexAttribDivisor(unsigned int index, unsigned int divisor);

        // ===== OpenGL Debug Functions =====
        static void EnableDebugOutput();
        static void SetDebugCallback();
        static void PushDebugGroup(const std::string& message);
        static void PopDebugGroup();
        static void ObjectLabel(GLenum identifier, unsigned int name, const std::string& label);

        // ===== OpenGL Performance Functions =====
        static void BeginQuery(GLenum target, unsigned int id);
        static void EndQuery(GLenum target);
        static int GetQueryObjecti(unsigned int id, GLenum pname);
        static void GetQueryObjectiv(unsigned int id, GLenum pname, int* params);
        static void GetQueryObjectuiv(unsigned int id, GLenum pname, unsigned int* params);

        // ===== OpenGL Synchronization =====
        static void FenceSync(GLenum condition, GLbitfield flags);
        static void ClientWaitSync(void* sync, GLbitfield flags, GLuint64 timeout);
        static void DeleteSync(void* sync);

    private:
        static void GLAPIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, 
                                           GLsizei length, const char* message, const void* userParam);
    };

} 