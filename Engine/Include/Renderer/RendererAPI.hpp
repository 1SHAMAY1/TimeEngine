#pragma once
#include "Renderer/GraphicsAPI.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace TE {

    enum class BlendFactor {
        Zero = 0,
        One,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstColor
    };

    class RendererAPI {
    public:
        virtual ~RendererAPI() = default;

        virtual void Init() = 0;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;
        virtual void DrawIndexed(uint32_t vao, uint32_t indexCount) = 0;
        virtual void SetBlendMode(int blendMode) = 0;

        virtual bool LoadLoader(void* (*loadProc)(const char*)) = 0;
        virtual std::string GetVersionString() = 0;
        virtual std::string GetGPUVendor() = 0;
        virtual std::string GetGPURenderer() = 0;

        virtual void GetViewport(int* viewport) = 0;
        virtual void GetClearColor(float* color) = 0;
        virtual void ReadPixelsRGBA(int x, int y, int width, int height, void* outPixels) = 0;
        virtual void SetBlendFunc(BlendFactor src, BlendFactor dst) = 0;
        virtual void SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha) = 0;

        static GraphicsAPI GetAPI();
        static std::unique_ptr<RendererAPI> Create();
    };

}
