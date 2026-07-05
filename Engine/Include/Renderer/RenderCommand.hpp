#pragma once
#include "Renderer/RendererAPI.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace TE {
	class RenderCommand {
	public:
		static void Init() { s_RendererAPI->Init(); }
		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) { s_RendererAPI->SetViewport(x, y, width, height); }
		static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
		static void Clear() { s_RendererAPI->Clear(); }
		static void DrawIndexed(uint32_t vao, uint32_t indexCount) { s_RendererAPI->DrawIndexed(vao, indexCount); }
		static void SetBlendMode(int blendMode) { s_RendererAPI->SetBlendMode(blendMode); }

		static bool LoadLoader(void* (*loadProc)(const char*)) { return s_RendererAPI->LoadLoader(loadProc); }
		static std::string GetVersionString() { return s_RendererAPI->GetVersionString(); }
		static std::string GetGPUVendor() { return s_RendererAPI->GetGPUVendor(); }
		static std::string GetGPURenderer() { return s_RendererAPI->GetGPURenderer(); }

		static void GetViewport(int* viewport) { s_RendererAPI->GetViewport(viewport); }
		static void GetClearColor(float* color) { s_RendererAPI->GetClearColor(color); }
		static void ReadPixelsRGBA(int x, int y, int width, int height, void* outPixels) { s_RendererAPI->ReadPixelsRGBA(x, y, width, height, outPixels); }
		static void SetBlendFunc(BlendFactor src, BlendFactor dst) { s_RendererAPI->SetBlendFunc(src, dst); }
		static void SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha) { s_RendererAPI->SetBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha); }

		static RendererAPI* GetAPIInstance() { return s_RendererAPI.get(); }

	private:
		static std::unique_ptr<RendererAPI> s_RendererAPI;
	};
}
