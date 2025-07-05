#include "Renderer/OpenGL/OpenGLShaderLibrary.hpp"
#include "Renderer/OpenGL/OpenGLShader.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace TE {

    // ===== OpenGL-Specific Shader Creation =====
    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLBasicShader() {
        return std::make_shared<OpenGLShader>(
            ShaderLibrary::GetBasicVertexShader(),
            ShaderLibrary::GetBasicFragmentShader()
        );
    }

    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLTextureShader() {
        return std::make_shared<OpenGLShader>(
            ShaderLibrary::GetTextureVertexShader(),
            ShaderLibrary::GetTextureFragmentShader()
        );
    }

    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLColorShader() {
        return std::make_shared<OpenGLShader>(
            ShaderLibrary::GetColorVertexShader(),
            ShaderLibrary::GetColorFragmentShader()
        );
    }

    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLLightingShader() {
        return std::make_shared<OpenGLShader>(
            ShaderLibrary::GetLightingVertexShader(),
            ShaderLibrary::GetLightingFragmentShader()
        );
    }

    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLParticleShader() {
        return std::make_shared<OpenGLShader>(
            ShaderLibrary::GetParticleVertexShader(),
            ShaderLibrary::GetParticleFragmentShader()
        );
    }

    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLPostProcessShader() {
        return std::make_shared<OpenGLShader>(
            ShaderLibrary::GetPostProcessVertexShader(),
            ShaderLibrary::GetPostProcessFragmentShader()
        );
    }

    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLUIShader() {
        return std::make_shared<OpenGLShader>(
            ShaderLibrary::GetUIVertexShader(),
            ShaderLibrary::GetUIFragmentShader()
        );
    }

    std::shared_ptr<OpenGLShader> OpenGLShaderLibrary::CreateOpenGLComputeShader(const std::string& computeSource) {
        return std::make_shared<OpenGLShader>(computeSource);
    }

    // ===== OpenGL-Specific Functions =====
    void OpenGLShaderLibrary::SetUniform1i(OpenGLShader* shader, const std::string& name, int value) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1) {
                glUniform1i(location, value);
            }
        }
    }

    void OpenGLShaderLibrary::SetUniform1f(OpenGLShader* shader, const std::string& name, float value) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1) {
                glUniform1f(location, value);
            }
        }
    }

    void OpenGLShaderLibrary::SetUniform2f(OpenGLShader* shader, const std::string& name, const glm::vec2& value) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1) {
                glUniform2f(location, value.x, value.y);
            }
        }
    }

    void OpenGLShaderLibrary::SetUniform3f(OpenGLShader* shader, const std::string& name, const glm::vec3& value) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1) {
                glUniform3f(location, value.x, value.y, value.z);
            }
        }
    }

    void OpenGLShaderLibrary::SetUniform4f(OpenGLShader* shader, const std::string& name, const glm::vec4& value) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1) {
                glUniform4f(location, value.x, value.y, value.z, value.w);
            }
        }
    }

    void OpenGLShaderLibrary::SetUniformMat3(OpenGLShader* shader, const std::string& name, const glm::mat3& value) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1) {
                glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    }

    void OpenGLShaderLibrary::SetUniformMat4(OpenGLShader* shader, const std::string& name, const glm::mat4& value) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1) {
                glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    }

    void OpenGLShaderLibrary::SetUniformMat4Array(OpenGLShader* shader, const std::string& name, const std::vector<glm::mat4>& values) {
        if (shader) {
            int location = shader->GetUniformLocation(name);
            if (location != -1 && !values.empty()) {
                glUniformMatrix4fv(location, (GLsizei)values.size(), GL_FALSE, glm::value_ptr(values[0]));
            }
        }
    }

    // ===== OpenGL State Management =====
    void OpenGLShaderLibrary::EnableBlending() {
        glEnable(GL_BLEND);
    }

    void OpenGLShaderLibrary::DisableBlending() {
        glDisable(GL_BLEND);
    }

    void OpenGLShaderLibrary::SetBlendFunc(GLenum srcFactor, GLenum dstFactor) {
        glBlendFunc(srcFactor, dstFactor);
    }

    void OpenGLShaderLibrary::EnableDepthTest() {
        glEnable(GL_DEPTH_TEST);
    }

    void OpenGLShaderLibrary::DisableDepthTest() {
        glDisable(GL_DEPTH_TEST);
    }

    void OpenGLShaderLibrary::SetDepthFunc(GLenum func) {
        glDepthFunc(func);
    }

    void OpenGLShaderLibrary::EnableStencilTest() {
        glEnable(GL_STENCIL_TEST);
    }

    void OpenGLShaderLibrary::DisableStencilTest() {
        glDisable(GL_STENCIL_TEST);
    }

    void OpenGLShaderLibrary::SetStencilFunc(GLenum func, int ref, unsigned int mask) {
        glStencilFunc(func, ref, mask);
    }

    void OpenGLShaderLibrary::SetStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
        glStencilOp(sfail, dpfail, dppass);
    }

    void OpenGLShaderLibrary::EnableCullFace() {
        glEnable(GL_CULL_FACE);
    }

    void OpenGLShaderLibrary::DisableCullFace() {
        glDisable(GL_CULL_FACE);
    }

    void OpenGLShaderLibrary::SetCullFace(GLenum face) {
        glCullFace(face);
    }

    void OpenGLShaderLibrary::SetFrontFace(GLenum mode) {
        glFrontFace(mode);
    }

    // ===== OpenGL Texture Functions =====
    void OpenGLShaderLibrary::BindTexture2D(unsigned int textureID, int slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void OpenGLShaderLibrary::BindTextureCube(unsigned int textureID, int slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    }

    void OpenGLShaderLibrary::SetTextureFiltering(unsigned int textureID, GLenum minFilter, GLenum magFilter) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    }

    void OpenGLShaderLibrary::SetTextureWrapping(unsigned int textureID, GLenum wrapS, GLenum wrapT) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    }

    void OpenGLShaderLibrary::GenerateMipmaps(unsigned int textureID) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // ===== OpenGL Framebuffer Functions =====
    unsigned int OpenGLShaderLibrary::CreateFramebuffer() {
        unsigned int framebuffer;
        glGenFramebuffers(1, &framebuffer);
        return framebuffer;
    }

    void OpenGLShaderLibrary::BindFramebuffer(unsigned int framebufferID) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    }

    void OpenGLShaderLibrary::AttachTexture2D(unsigned int framebufferID, unsigned int textureID, GLenum attachment) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureID, 0);
    }

    void OpenGLShaderLibrary::AttachRenderbuffer(unsigned int framebufferID, unsigned int renderbufferID, GLenum attachment) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbufferID);
    }

    void OpenGLShaderLibrary::SetDrawBuffers(const std::vector<GLenum>& attachments) {
        glDrawBuffers((GLsizei)attachments.size(), attachments.data());
    }

    bool OpenGLShaderLibrary::CheckFramebufferStatus(unsigned int framebufferID) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
        return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    void OpenGLShaderLibrary::DeleteFramebuffer(unsigned int framebufferID) {
        glDeleteFramebuffers(1, &framebufferID);
    }

    // ===== OpenGL Compute Shader Functions =====
    void OpenGLShaderLibrary::DispatchCompute(unsigned int shaderID, unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) {
        glUseProgram(shaderID);
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    }

    void OpenGLShaderLibrary::MemoryBarrier(GLenum barriers) {
        glMemoryBarrier(barriers);
    }

    void OpenGLShaderLibrary::BindImageTexture(unsigned int unit, unsigned int textureID, int level, bool layered, int layer, GLenum access, GLenum format) {
        glBindImageTexture(unit, textureID, level, layered, layer, access, format);
    }

    // ===== OpenGL Uniform Buffer Functions =====
    unsigned int OpenGLShaderLibrary::CreateUniformBuffer(const void* data, size_t size) {
        unsigned int ubo;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
        return ubo;
    }

    void OpenGLShaderLibrary::BindUniformBuffer(unsigned int uboID, unsigned int bindingPoint) {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uboID);
    }

    void OpenGLShaderLibrary::UpdateUniformBuffer(unsigned int uboID, const void* data, size_t offset, size_t size) {
        glBindBuffer(GL_UNIFORM_BUFFER, uboID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    }

    void OpenGLShaderLibrary::BindUniformBlock(OpenGLShader* shader, const std::string& blockName, unsigned int bindingPoint) {
        unsigned int blockIndex = glGetUniformBlockIndex(shader->GetRendererID(), blockName.c_str());
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(shader->GetRendererID(), blockIndex, bindingPoint);
        }
    }

    // ===== OpenGL Vertex Array Functions =====
    void OpenGLShaderLibrary::SetVertexAttribPointer(unsigned int index, int size, GLenum type, bool normalized, int stride, const void* pointer) {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }

    void OpenGLShaderLibrary::EnableVertexAttribArray(unsigned int index) {
        glEnableVertexAttribArray(index);
    }

    void OpenGLShaderLibrary::DisableVertexAttribArray(unsigned int index) {
        glDisableVertexAttribArray(index);
    }

    void OpenGLShaderLibrary::VertexAttribDivisor(unsigned int index, unsigned int divisor) {
        glVertexAttribDivisor(index, divisor);
    }

    // ===== OpenGL Debug Functions =====
    void OpenGLShaderLibrary::EnableDebugOutput() {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

    void OpenGLShaderLibrary::SetDebugCallback() {
        glDebugMessageCallback(DebugCallback, nullptr);
    }

    void OpenGLShaderLibrary::PushDebugGroup(const std::string& message) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, (GLsizei)message.length(), message.c_str());
    }

    void OpenGLShaderLibrary::PopDebugGroup() {
        glPopDebugGroup();
    }

    void OpenGLShaderLibrary::ObjectLabel(GLenum identifier, unsigned int name, const std::string& label) {
        glObjectLabel(identifier, name, (GLsizei)label.length(), label.c_str());
    }

    // ===== OpenGL Performance Functions =====
    void OpenGLShaderLibrary::BeginQuery(GLenum target, unsigned int id) {
        glBeginQuery(target, id);
    }

    void OpenGLShaderLibrary::EndQuery(GLenum target) {
        glEndQuery(target);
    }

    int OpenGLShaderLibrary::GetQueryObjecti(unsigned int id, GLenum pname) {
        int result;
        glGetQueryObjectiv(id, pname, &result);
        return result;
    }

    void OpenGLShaderLibrary::GetQueryObjectiv(unsigned int id, GLenum pname, int* params) {
        glGetQueryObjectiv(id, pname, params);
    }

    void OpenGLShaderLibrary::GetQueryObjectuiv(unsigned int id, GLenum pname, unsigned int* params) {
        glGetQueryObjectuiv(id, pname, params);
    }

    // ===== OpenGL Synchronization =====
    void OpenGLShaderLibrary::FenceSync(GLenum condition, GLbitfield flags) {
        glFenceSync(condition, flags);
    }

    void OpenGLShaderLibrary::ClientWaitSync(void* sync, GLbitfield flags, GLuint64 timeout) {
        glClientWaitSync((GLsync)sync, flags, timeout);
    }

    void OpenGLShaderLibrary::DeleteSync(void* sync) {
        glDeleteSync((GLsync)sync);
    }

    // ===== Debug Callback =====
    void GLAPIENTRY OpenGLShaderLibrary::DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, 
                                                       GLsizei length, const char* message, const void* userParam) {
        // Ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        std::string sourceStr;
        switch (source) {
        case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "WINDOW_SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "SHADER_COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "THIRD_PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER: sourceStr = "OTHER"; break;
        }

        std::string typeStr;
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: typeStr = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "UNDEFINED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_MARKER: typeStr = "MARKER"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "PUSH_GROUP"; break;
        case GL_DEBUG_TYPE_POP_GROUP: typeStr = "POP_GROUP"; break;
        case GL_DEBUG_TYPE_OTHER: typeStr = "OTHER"; break;
        }

        std::string severityStr;
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: severityStr = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW: severityStr = "LOW"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "NOTIFICATION"; break;
        }

        std::cout << "OpenGL Debug [" << sourceStr << "][" << typeStr << "][" << severityStr << "] " << id << ": " << message << std::endl;
    }

} 