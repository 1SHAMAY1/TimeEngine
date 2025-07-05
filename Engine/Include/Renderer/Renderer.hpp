#pragma once
#include <glm/glm.hpp>

namespace TE {

	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);
		static void BeginFrame();
		static void EndFrame();

		static void Clear(const glm::vec4& color);
	};
}
