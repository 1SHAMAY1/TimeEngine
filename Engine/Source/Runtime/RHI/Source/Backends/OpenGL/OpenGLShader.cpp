#include "PreRequisites.h"
#include "Renderer/OpenGL/OpenGLShader.hpp"
#include "Log.h"
#include <glad/glad.h>
#include <iostream>

static uint32_t CompileShader(uint32_t type, const TEString &source)
{
    if (!glad_glCreateShader)
    {
        return 0;
    }

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
        TE_CORE_ERROR("Shader compilation failed: {0}", infoLog.c_str());
    }

    return shader;
}

OpenGLShader::OpenGLShader(const TEString &vertexSrc, const TEString &fragmentSrc)
{
    if (!glad_glCreateProgram)
    {
        m_RendererID = 0;
        return;
    }

    uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

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
        TE_CORE_ERROR("Shader linking failed: {0}", infoLog.c_str());
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

OpenGLShader::OpenGLShader(const TEString &computeSrc)
{
    if (!glad_glCreateProgram)
    {
        m_RendererID = 0;
        return;
    }

    uint32_t computeShader = CompileShader(GL_COMPUTE_SHADER, computeSrc);

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, computeShader);
    glLinkProgram(m_RendererID);

    int success;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
    if (!success)
    {
        TEString infoLog;
        infoLog.Reserve(512);
        glGetProgramInfoLog(m_RendererID, 512, nullptr, infoLog.Data());
        TE_CORE_ERROR("Compute shader linking failed: {0}", infoLog.c_str());
    }

    glDeleteShader(computeShader);
}

OpenGLShader::~OpenGLShader()
{
    if (glad_glDeleteProgram && m_RendererID != 0)
    {
        glDeleteProgram(m_RendererID);
    }
}

void OpenGLShader::Bind() const
{
    if (glad_glUseProgram && m_RendererID != 0)
    {
        glUseProgram(m_RendererID);
    }
}

void OpenGLShader::Unbind() const
{
    if (glad_glUseProgram)
    {
        glUseProgram(0);
    }
}

int OpenGLShader::GetUniformLocation(const TEString &name) { return glGetUniformLocation(m_RendererID, name.c_str()); }

void OpenGLShader::SetUniformMat4(const TEString &name, const TEMatrix4 &value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value.m[0][0]);
}

void OpenGLShader::SetUniform4f(const TEString &name, const TEVector4 &value)
{
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void OpenGLShader::SetUniform3f(const TEString &name, const TEVector &value)
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void OpenGLShader::SetUniform2f(const TEString &name, const TEVector2 &value)
{
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void OpenGLShader::SetUniform1f(const TEString &name, float value) { glUniform1f(GetUniformLocation(name), value); }

void OpenGLShader::SetUniform1i(const TEString &name, int value) { glUniform1i(GetUniformLocation(name), value); }
