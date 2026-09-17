#include "PreRequisites.h"
#include "Renderer/DirectX11/DirectX11ShaderLibrary.hpp"
#include "RendererContext.hpp"

#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)

// ===== Factory Methods =====
TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11BasicShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLBasicVertexShader(), GetHLSLBasicPixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11TextureShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLTextureVertexShader(), GetHLSLTexturePixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11ColorShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLColorVertexShader(), GetHLSLColorPixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11StandardShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLColorVertexShader(), GetHLSLColorPixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11LightingShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLBasicVertexShader(), GetHLSLBasicPixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11ParticleShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLBasicVertexShader(), GetHLSLBasicPixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11PostProcessShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLTextureVertexShader(), GetHLSLTexturePixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11UIShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLTextureVertexShader(), GetHLSLTexturePixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11Light2DShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLLight2DVertexShader(), GetHLSLLight2DPixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11AmbientGradientShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLLight2DVertexShader(), GetHLSLAmbientGradientPixelShader());
}

TERef<DirectX11Shader> DirectX11ShaderLibrary::CreateDirectX11LightBlendShader()
{
    return CreateRef<DirectX11Shader>(GetHLSLLightBlendVertexShader(), GetHLSLLightBlendPixelShader());
}

// ===== HLSL Shader Definitions =====

TEString DirectX11ShaderLibrary::GetHLSLBasicVertexShader()
{
    return R"(
cbuffer PerDraw : register(b0)
{
    float4x4 u_Transform;
    float4x4 u_ViewProjection;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = mul(u_ViewProjection, mul(u_Transform, float4(input.Position, 1.0f)));
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;
    return output;
}
)";
}

TEString DirectX11ShaderLibrary::GetHLSLBasicPixelShader()
{
    return R"(
cbuffer PerMaterial : register(b0)
{
    float4 u_Color;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.Color * u_Color;
}
)";
}

TEString DirectX11ShaderLibrary::GetHLSLTextureVertexShader()
{
    return R"(
cbuffer PerDraw : register(b0)
{
    float4x4 u_Transform;
    float4x4 u_ViewProjection;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = mul(u_ViewProjection, mul(u_Transform, float4(input.Position, 1.0f)));
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;
    return output;
}
)";
}

TEString DirectX11ShaderLibrary::GetHLSLTexturePixelShader()
{
    return R"(
Texture2D    g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

cbuffer PerMaterial : register(b0)
{
    float4 u_Color;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return g_Texture.Sample(g_Sampler, input.TexCoord) * input.Color * u_Color;
}
)";
}

TEString DirectX11ShaderLibrary::GetHLSLColorVertexShader() { return GetHLSLBasicVertexShader(); }

TEString DirectX11ShaderLibrary::GetHLSLColorPixelShader() { return GetHLSLBasicPixelShader(); }

TEString DirectX11ShaderLibrary::GetHLSLLight2DVertexShader()
{
    return R"(
cbuffer PerDraw : register(b0)
{
    float4x4 u_Transform;
    float4x4 u_ViewProjection;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 LocalPos : TEXCOORD0;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = mul(u_ViewProjection, mul(u_Transform, float4(input.Position, 1.0f)));
    output.LocalPos = input.Position.xy;
    return output;
}
)";
}

TEString DirectX11ShaderLibrary::GetHLSLLight2DPixelShader()
{
    return R"(
cbuffer LightParams : register(b0)
{
    float4 u_Color;
    float  u_Radius;
    float  u_Intensity;
    float  u_FalloffExponent;
    int    u_LightType;
    float3 u_Direction;
    float  u_InnerSpotAngle;
    float  u_OuterSpotAngle;
    float  u_LineLength;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 LocalPos : TEXCOORD0;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float dist = length(input.LocalPos);
    if (dist > 1.0f)
        discard;

    float falloff = pow(saturate(1.0f - dist), max(u_FalloffExponent, 0.001f)) * u_Intensity;
    return float4(u_Color.rgb * falloff, falloff);
}
)";
}

TEString DirectX11ShaderLibrary::GetHLSLAmbientGradientPixelShader()
{
    return R"(
cbuffer AmbientParams : register(b0)
{
    float4 u_SkyColor;
    float4 u_HorizonColor;
    float4 u_GroundColor;
    float  u_Intensity;
    float  u_HorizonHeight;
    float  u_HorizonSpread;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 LocalPos : TEXCOORD0;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float y = input.LocalPos.y * 0.5f + 0.5f;
    float3 color;
    if (y > u_HorizonHeight)
    {
        float t = saturate((y - u_HorizonHeight) / max(u_HorizonSpread, 0.0001f));
        color = lerp(u_HorizonColor.rgb, u_SkyColor.rgb, t);
    }
    else
    {
        float t = saturate((u_HorizonHeight - y) / max(u_HorizonSpread, 0.0001f));
        color = lerp(u_HorizonColor.rgb, u_GroundColor.rgb, t);
    }
    return float4(color * u_Intensity, 1.0f);
}
)";
}

TEString DirectX11ShaderLibrary::GetHLSLLightBlendVertexShader() { return GetHLSLLight2DVertexShader(); }

TEString DirectX11ShaderLibrary::GetHLSLLightBlendPixelShader()
{
    return R"(
Texture2D    u_LightTexture : register(t0);
SamplerState g_Sampler      : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 LocalPos : TEXCOORD0;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float2 uv = input.LocalPos * 0.5f + 0.5f;
    return u_LightTexture.Sample(g_Sampler, uv);
}
)";
}

#endif // TE_SUPPORT_DIRECTX11 && TE_PLATFORM_WINDOWS
