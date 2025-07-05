#pragma once
#include <glm/glm.hpp>

namespace TE {
	class RenderCommand {
	public:
		static void Init();
		static void SetClearColor(const glm::vec4& color);
		static void Clear();
		static void DrawIndexed(unsigned int vao, unsigned int indexCount);
	};
}
