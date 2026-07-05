#pragma once
#include "Renderer/RendererAPI.hpp"

// Forward-declare D3D11 COM types to avoid pulling windows.h into headers.
// These are pointer-only uses so incomplete types are sufficient.
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11BlendState;

namespace TE {

    // Central D3D11 device context shared by all DX11 resource classes.
    // Populated by DirectX11RendererAPI::InitWithWindow().
    struct DX11Context {
        ID3D11Device*           Device          = nullptr;
        ID3D11DeviceContext*    DeviceContext    = nullptr;
        IDXGISwapChain*         SwapChain        = nullptr;
        ID3D11RenderTargetView* RenderTargetView = nullptr;
        ID3D11DepthStencilView* DepthStencilView = nullptr;
        ID3D11Texture2D*        DepthStencilTex  = nullptr;

        static DX11Context& Get() {
            static DX11Context s_Instance;
            return s_Instance;
        }
    };

    class DirectX11RendererAPI : public RendererAPI {
    public:
        virtual ~DirectX11RendererAPI() override;

        // Init() satisfies the RendererAPI interface (no-op for D3D11).
        // Call InitWithWindow() after the GLFW window is created to build the swap chain.
        virtual void Init() override;
        void InitWithWindow(void* hwnd, uint32_t width, uint32_t height);

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;
        virtual void DrawIndexed(uint32_t vao, uint32_t indexCount) override;
        virtual void SetBlendMode(int blendMode) override;

        virtual bool LoadLoader(void* (*loadProc)(const char*)) override;
        virtual std::string GetVersionString() override;
        virtual std::string GetGPUVendor() override;
        virtual std::string GetGPURenderer() override;

        virtual void GetViewport(int* viewport) override;
        virtual void GetClearColor(float* color) override;
        virtual void ReadPixelsRGBA(int x, int y, int width, int height, void* outPixels) override;
        virtual void SetBlendFunc(BlendFactor src, BlendFactor dst) override;
        virtual void SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB,
                                          BlendFactor srcAlpha, BlendFactor dstAlpha) override;

    private:
        void CreateRenderTargetView();
        void CreateDepthStencilView(uint32_t width, uint32_t height);
        void ReleaseRenderTargets();
        // NOTE: ToDXBlend is a file-local free function in the .cpp (requires D3D11_BLEND from d3d11.h)

        glm::vec4   m_ClearColor     = { 0.1f, 0.1f, 0.1f, 1.0f };
        int         m_ViewportX      = 0;
        int         m_ViewportY      = 0;
        uint32_t    m_ViewportW      = 0;
        uint32_t    m_ViewportH      = 0;
        std::string m_GPUDescription;
    };

}
