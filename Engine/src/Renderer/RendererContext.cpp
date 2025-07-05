#include "Renderer/RendererContext.hpp"

namespace TE {
	GraphicsAPI RendererContext::s_API = GraphicsAPI::OpenGL;

	void RendererContext::SetAPI(GraphicsAPI api) {
		s_API = api;
	}

	GraphicsAPI RendererContext::GetAPI() {
		return s_API;
	}
}