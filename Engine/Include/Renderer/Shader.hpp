#pragma once
#include "Core/PreRequisites.h"
#include <glm/glm.hpp>

namespace TE {
	class Shader {
	public:
		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);

		virtual ~Shader() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetUniformMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetUniform4f(const std::string& name, const glm::vec4& value) = 0;
	};
}

