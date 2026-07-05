#include "Renderer/OpenGLES/OpenGLESShader.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace TE
{

static uint32_t CompileGLESShader(uint32_t type, const std::string &source)
{
    uint32_t shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "[OpenGL ES] Shader compilation failed:\n" << infoLog << std::endl;
    }

    return shader;
}

OpenGLESShader::OpenGLESShader(const std::string &vertexSrc, const std::string &fragmentSrc)
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
        char infoLog[512];
        glGetProgramInfoLog(m_RendererID, 512, nullptr, infoLog);
        std::cerr << "[OpenGL ES] Shader linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

OpenGLESShader::~OpenGLESShader() { glDeleteProgram(m_RendererID); }

void OpenGLESShader::Bind() const { glUseProgram(m_RendererID); }

void OpenGLESShader::Unbind() const { glUseProgram(0); }

int OpenGLESShader::GetUniformLocation(const std::string &name)
{
    return glGetUniformLocation(m_RendererID, name.c_str());
}

void OpenGLESShader::SetUniformMat4(const std::string &name, const glm::mat4 &value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGLESShader::SetUniform4f(const std::string &name, const glm::vec4 &value)
{
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void OpenGLESShader::SetUniform3f(const std::string &name, const glm::vec3 &value)
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void OpenGLESShader::SetUniform2f(const std::string &name, const glm::vec2 &value)
{
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void OpenGLESShader::SetUniform1f(const std::string &name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void OpenGLESShader::SetUniform1i(const std::string &name, int value) { glUniform1i(GetUniformLocation(name), value); }

} // namespace TE
