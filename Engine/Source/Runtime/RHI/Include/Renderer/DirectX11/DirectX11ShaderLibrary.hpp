#pragma once
#include "PreRequisites.h"
#include "Renderer/DirectX11/DirectX11Shader.hpp"
#include "ShaderLibrary.hpp"

#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)

class DirectX11ShaderLibrary : public ShaderLibrary
{
public:
    // ===== DirectX11-Specific Shader Creation =====
    static TERef<DirectX11Shader> CreateDirectX11BasicShader();
    static TERef<DirectX11Shader> CreateDirectX11TextureShader();
    static TERef<DirectX11Shader> CreateDirectX11ColorShader();
    static TERef<DirectX11Shader> CreateDirectX11StandardShader();
    static TERef<DirectX11Shader> CreateDirectX11LightingShader();
    static TERef<DirectX11Shader> CreateDirectX11ParticleShader();
    static TERef<DirectX11Shader> CreateDirectX11PostProcessShader();
    static TERef<DirectX11Shader> CreateDirectX11UIShader();
    static TERef<DirectX11Shader> CreateDirectX11Light2DShader();
    static TERef<DirectX11Shader> CreateDirectX11AmbientGradientShader();
    static TERef<DirectX11Shader> CreateDirectX11LightBlendShader();

    // ===== HLSL Shader Sources =====
    static TEString GetHLSLBasicVertexShader();
    static TEString GetHLSLBasicPixelShader();

    static TEString GetHLSLTextureVertexShader();
    static TEString GetHLSLTexturePixelShader();

    static TEString GetHLSLColorVertexShader();
    static TEString GetHLSLColorPixelShader();

    static TEString GetHLSLLight2DVertexShader();
    static TEString GetHLSLLight2DPixelShader();

    static TEString GetHLSLAmbientGradientPixelShader();
    static TEString GetHLSLLightBlendVertexShader();
    static TEString GetHLSLLightBlendPixelShader();
};

#endif // TE_SUPPORT_DIRECTX11 && TE_PLATFORM_WINDOWS
