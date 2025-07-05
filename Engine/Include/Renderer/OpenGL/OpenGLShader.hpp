#pragma once
#include "Renderer/Shader.hpp"

namespace TE {
    class OpenGLShader : public Shader {
    public:
        OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetUniformMat4(const std::string& name, const glm::mat4& value) override;
        virtual void SetUniform4f(const std::string& name, const glm::vec4& value) override;

    private:
        uint32_t m_RendererID;

        int GetUniformLocation(const std::string& name);
    };
}
