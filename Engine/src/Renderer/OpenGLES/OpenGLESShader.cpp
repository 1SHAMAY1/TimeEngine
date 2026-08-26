#include "Renderer/OpenGLES/OpenGLESShader.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

static uint32_t CompileGLESShader(uint32_t type, const TEString &source)
{
    uint32_t shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        TEString infoLog;
        infoLog.Reserve(512);
        glGetShaderInfoLog(shader, 512, nullptr, infoLog.Data());
        TE_CORE_ERROR("[OpenGL ES] Shader compilation failed: {0}", infoLog.c_str());
    }

    return shader;
}

OpenGLESShader::OpenGLESShader(const TEString &vertexSrc, const TEString &fragmentSrc)
{
    uint32_t vertexShader = CompileGLESShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fragmentShader = CompileGLESShader(GL_FRAGMENT_SHADER, fragmentSrc);

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vertexShader);
    glAttachShader(m_RendererID, fragmentShader);
    glLinkProgram(m_RendererID);

    int success;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
    if (!success)
    {
        TEString infoLog;
        infoLog.Reserve(512);
        glGetProgramInfoLog(m_RendererID, 512, nullptr, infoLog.Data());
        TE_CORE_ERROR("[OpenGL ES] Shader linking failed: {0}", infoLog.c_str());
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

OpenGLESShader::~OpenGLESShader() { glDeleteProgram(m_RendererID); }

void OpenGLESShader::Bind() const { glUseProgram(m_RendererID); }

void OpenGLESShader::Unbind() const { glUseProgram(0); }

int OpenGLESShader::GetUniformLocation(const TEString &name)
{
    return glGetUniformLocation(m_RendererID, name.c_str());
}

void OpenGLESShader::SetUniformMat4(const TEString &name, const glm::mat4 &value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGLESShader::SetUniform4f(const TEString &name, const glm::vec4 &value)
{
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void OpenGLESShader::SetUniform3f(const TEString &name, const glm::vec3 &value)
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void OpenGLESShader::SetUniform2f(const TEString &name, const glm::vec2 &value)
{
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void OpenGLESShader::SetUniform1f(const TEString &name, float value) { glUniform1f(GetUniformLocation(name), value); }

void OpenGLESShader::SetUniform1i(const TEString &name, int value) { glUniform1i(GetUniformLocation(name), value); }
