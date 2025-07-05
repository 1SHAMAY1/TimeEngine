#include "Renderer/OpenGL/OpenGLShader.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace TE {
	static uint32_t CompileShader(uint32_t type, const std::string& source) {
		uint32_t shader = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
		}

		return shader;
	}

	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
		uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
		uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, vertexShader);
		glAttachShader(m_RendererID, fragmentShader);
		glLinkProgram(m_RendererID);

		int success;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(m_RendererID, 512, nullptr, infoLog);
			std::cerr << "Shader linking failed:\n" << infoLog << std::endl;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	OpenGLShader::OpenGLShader(const std::string& computeSrc) {
		uint32_t computeShader = CompileShader(GL_COMPUTE_SHADER, computeSrc);

		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, computeShader);
		glLinkProgram(m_RendererID);

		int success;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(m_RendererID, 512, nullptr, infoLog);
			std::cerr << "Compute shader linking failed:\n" << infoLog << std::endl;
		}

		glDeleteShader(computeShader);
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}

	int OpenGLShader::GetUniformLocation(const std::string& name) {
		return glGetUniformLocation(m_RendererID, name.c_str());
	}

	void OpenGLShader::SetUniformMat4(const std::string& name, const glm::mat4& value) {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::SetUniform4f(const std::string& name, const glm::vec4& value) {
		glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
	}
}
