#include "Core/PreRequisites.h"
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/GraphicsAPI.hpp"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLShader.hpp"
#include "Renderer/OpenGL/OpenGLShaderLibrary.hpp"
#endif
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESShader.hpp"
#include "Renderer/OpenGLES/OpenGLESShaderLibrary.hpp"
#endif
#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanShader.hpp"
#include "Renderer/Vulkan/VulkanShaderLibrary.hpp"
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
#include "Renderer/DirectX11/DirectX11Shader.hpp"
#include "Renderer/DirectX11/DirectX11ShaderLibrary.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#include "Renderer/Shader.hpp"
#include "Utils/Math/MathEngine.hpp"

TEMap<TEString, TERef<Shader>> ShaderLibrary::s_ShaderCache;

// ===== Shader Creation =====
TERef<Shader> ShaderLibrary::CreateBasicShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLBasicShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESBasicShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanBasicShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11BasicShader();
#endif
    default:
        return Shader::Create(GetBasicVertexShader(), GetBasicFragmentShader());
    }
}

TERef<Shader> ShaderLibrary::CreateTextureShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLTextureShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESTextureShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanTextureShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11TextureShader();
#endif
    default:
        return Shader::Create(GetTextureVertexShader(), GetTextureFragmentShader());
    }
}

TERef<Shader> ShaderLibrary::CreateColorShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLColorShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESColorShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanColorShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11ColorShader();
#endif
    default:
        return Shader::Create(GetColorVertexShader(), GetColorFragmentShader());
    }
}

TERef<Shader> ShaderLibrary::CreateStandardShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLStandardShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESStandardShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanStandardShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11StandardShader();
#endif
    default:
        return Shader::Create(GetColorVertexShader(), GetStandardFragmentShader());
    }
}

TERef<Shader> ShaderLibrary::CreateLightingShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLLightingShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESLightingShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanLightingShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11LightingShader();
#endif
    default:
        return Shader::Create(GetLightingVertexShader(), GetLightingFragmentShader());
    }
}

TERef<Shader> ShaderLibrary::CreateLight2DShader()
{
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
        return DirectX11ShaderLibrary::CreateDirectX11Light2DShader();
#endif
    return Shader::Create(GetLight2DVertexShader(), GetLight2DFragmentShader());
}

TERef<Shader> ShaderLibrary::CreateAmbientGradientShader()
{
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
        return DirectX11ShaderLibrary::CreateDirectX11AmbientGradientShader();
#endif
    return Shader::Create(GetLight2DVertexShader(), GetAmbientGradientFragmentShader());
}

TERef<Shader> ShaderLibrary::CreateLightBlendShader()
{
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
        return DirectX11ShaderLibrary::CreateDirectX11LightBlendShader();
#endif
    return Shader::Create(GetLightBlendVertexShader(), GetLightBlendFragmentShader());
}

TERef<Shader> ShaderLibrary::CreateParticleShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLParticleShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESParticleShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanParticleShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11ParticleShader();
#endif
    default:
        return Shader::Create(GetParticleVertexShader(), GetParticleFragmentShader());
    }
}

TERef<Shader> ShaderLibrary::CreatePostProcessShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLPostProcessShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESPostProcessShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanPostProcessShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11PostProcessShader();
#endif
    default:
        return Shader::Create(GetPostProcessVertexShader(), GetPostProcessFragmentShader());
    }
}

TERef<Shader> ShaderLibrary::CreateUIShader()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return OpenGLShaderLibrary::CreateOpenGLUIShader();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return OpenGLESShaderLibrary::CreateOpenGLESUIShader();
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return VulkanShaderLibrary::CreateVulkanUIShader();
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
    case GraphicsAPI::DirectX11:
        return DirectX11ShaderLibrary::CreateDirectX11UIShader();
#endif
    default:
        return Shader::Create(GetUIVertexShader(), GetUIFragmentShader());
    }
}

// ===== Common Shader Functions =====
void ShaderLibrary::SetMVP(Shader *shader, const TEMatrix4 &model, const TEMatrix4 &view, const TEMatrix4 &projection)
{
    if (shader)
    {
        shader->SetUniformMat4("u_Model", model);
        shader->SetUniformMat4("u_View", view);
        shader->SetUniformMat4("u_Projection", projection);
        shader->SetUniformMat4("u_MVP", projection * view * model);
    }
}

void ShaderLibrary::SetColor(Shader *shader, const TEColor &color)
{
    SetColor(shader, TEVector4(color.GetR(), color.GetG(), color.GetB(), color.GetA()));
}

void ShaderLibrary::SetColor(Shader *shader, const TEVector4 &color)
{
    if (shader)
    {
        shader->SetUniform4f("u_Color", color);
    }
}

void ShaderLibrary::SetTransform(Shader *shader, const TEMatrix4 &transform)
{
    if (shader)
    {
        shader->SetUniformMat4("u_Transform", transform);
    }
}

void ShaderLibrary::SetViewProjection(Shader *shader, const TEMatrix4 &viewProjection)
{
    if (shader)
    {
        shader->SetUniformMat4("u_ViewProjection", viewProjection);
    }
}

void ShaderLibrary::SetLightPosition(Shader *shader, const TEVector &position)
{
    if (shader)
    {
        shader->SetUniform3f("u_LightPosition", position);
    }
}

void ShaderLibrary::SetLightColor(Shader *shader, const TEColor &color)
{
    if (shader)
    {
        shader->SetUniform4f("u_LightColor", TEVector4(color.GetR(), color.GetG(), color.GetB(), color.GetA()));
    }
}

void ShaderLibrary::SetAmbientLight(Shader *shader, float intensity)
{
    if (shader)
    {
        shader->SetUniform1f("u_AmbientIntensity", intensity);
    }
}

void ShaderLibrary::SetDiffuseLight(Shader *shader, float intensity)
{
    if (shader)
    {
        shader->SetUniform1f("u_DiffuseIntensity", intensity);
    }
}

void ShaderLibrary::SetSpecularLight(Shader *shader, float intensity, float shininess)
{
    if (shader)
    {
        shader->SetUniform1f("u_SpecularIntensity", intensity);
        shader->SetUniform1f("u_Shininess", shininess);
    }
}

void ShaderLibrary::SetTexture(Shader *shader, int slot)
{
    if (shader)
    {
        shader->SetUniform1i("u_Texture", slot);
    }
}

void ShaderLibrary::SetTime(Shader *shader, float time)
{
    if (shader)
    {
        shader->SetUniform1f("u_Time", time);
    }
}

void ShaderLibrary::SetResolution(Shader *shader, const TEVector2 &resolution)
{
    if (shader)
    {
        shader->SetUniform2f("u_Resolution", resolution);
    }
}

void ShaderLibrary::SetCameraPosition(Shader *shader, const TEVector &position)
{
    if (shader)
    {
        shader->SetUniform3f("u_CameraPosition", position);
    }
}

void ShaderLibrary::SetFog(Shader *shader, const TEColor &color, float density, float start, float end)
{
    if (shader)
    {
        shader->SetUniform4f("u_FogColor", TEVector4(color.GetR(), color.GetG(), color.GetB(), color.GetA()));
        shader->SetUniform1f("u_FogDensity", density);
        shader->SetUniform1f("u_FogStart", start);
        shader->SetUniform1f("u_FogEnd", end);
    }
}

// ===== Utility Functions =====
TEMatrix4 ShaderLibrary::CreateModelMatrix(const TEVector &position, const TEVector &rotation, const TEVector &scale)
{
    float degToRad = 3.14159265358979323846f / 180.0f;
    TEMatrix4 model = MathEngine::Get().GetActiveAPI()->Translate(TEMatrix4(1.0f), position);
    model = MathEngine::Get().GetActiveAPI()->Rotate(model, rotation.x * degToRad, TEVector(1.0f, 0.0f, 0.0f));
    model = MathEngine::Get().GetActiveAPI()->Rotate(model, rotation.y * degToRad, TEVector(0.0f, 1.0f, 0.0f));
    model = MathEngine::Get().GetActiveAPI()->Rotate(model, rotation.z * degToRad, TEVector(0.0f, 0.0f, 1.0f));
    model = MathEngine::Get().GetActiveAPI()->Scale(model, scale);
    return model;
}

TEMatrix4 ShaderLibrary::CreateViewMatrix(const TEVector &position, const TEVector &target, const TEVector &up)
{
    return MathEngine::Get().GetActiveAPI()->LookAt(position, target, up);
}

TEMatrix4 ShaderLibrary::CreateProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
    float fovRad = fov * (3.14159265358979323846f / 180.0f);
    return MathEngine::Get().GetActiveAPI()->Perspective(fovRad, aspectRatio, nearPlane, farPlane);
}

TEMatrix4 ShaderLibrary::CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane,
                                                  float farPlane)
{
    return MathEngine::Get().GetActiveAPI()->Ortho(left, right, bottom, top, nearPlane, farPlane);
}

// ===== Color Utilities =====
void ShaderLibrary::SetTint(Shader *shader, const TEColor &tint, float intensity)
{
    if (shader)
    {
        shader->SetUniform4f("u_Tint", TEVector4(tint.GetR(), tint.GetG(), tint.GetB(), tint.GetA()));
        shader->SetUniform1f("u_TintIntensity", intensity);
    }
}

void ShaderLibrary::SetBrightness(Shader *shader, float brightness)
{
    if (shader)
    {
        shader->SetUniform1f("u_Brightness", brightness);
    }
}

void ShaderLibrary::SetContrast(Shader *shader, float contrast)
{
    if (shader)
    {
        shader->SetUniform1f("u_Contrast", contrast);
    }
}

void ShaderLibrary::SetSaturation(Shader *shader, float saturation)
{
    if (shader)
    {
        shader->SetUniform1f("u_Saturation", saturation);
    }
}

void ShaderLibrary::SetGamma(Shader *shader, float gamma)
{
    if (shader)
    {
        shader->SetUniform1f("u_Gamma", gamma);
    }
}

// ===== Material Properties =====
void ShaderLibrary::SetMaterial(Shader *shader, const TEColor &ambient, const TEColor &diffuse, const TEColor &specular,
                                float shininess)
{
    if (shader)
    {
        shader->SetUniform4f("u_Material.ambient",
                             TEVector4(ambient.GetR(), ambient.GetG(), ambient.GetB(), ambient.GetA()));
        shader->SetUniform4f("u_Material.diffuse",
                             TEVector4(diffuse.GetR(), diffuse.GetG(), diffuse.GetB(), diffuse.GetA()));
        shader->SetUniform4f("u_Material.specular",
                             TEVector4(specular.GetR(), specular.GetG(), specular.GetB(), specular.GetA()));
        shader->SetUniform1f("u_Material.shininess", shininess);
    }
}

void ShaderLibrary::SetEmissive(Shader *shader, const TEColor &emissive)
{
    if (shader)
    {
        shader->SetUniform4f("u_Emissive",
                             TEVector4(emissive.GetR(), emissive.GetG(), emissive.GetB(), emissive.GetA()));
    }
}

void ShaderLibrary::SetMetallic(Shader *shader, float metallic)
{
    if (shader)
    {
        shader->SetUniform1f("u_Metallic", metallic);
    }
}

void ShaderLibrary::SetRoughness(Shader *shader, float roughness)
{
    if (shader)
    {
        shader->SetUniform1f("u_Roughness", roughness);
    }
}

void ShaderLibrary::SetNormalMap(Shader *shader, int slot)
{
    if (shader)
    {
        shader->SetUniform1i("u_NormalMap", slot);
    }
}

void ShaderLibrary::SetRoughnessMap(Shader *shader, int slot)
{
    if (shader)
    {
        shader->SetUniform1i("u_RoughnessMap", slot);
    }
}

void ShaderLibrary::SetMetallicMap(Shader *shader, int slot)
{
    if (shader)
    {
        shader->SetUniform1i("u_MetallicMap", slot);
    }
}

void ShaderLibrary::SetAOMap(Shader *shader, int slot)
{
    if (shader)
    {
        shader->SetUniform1i("u_AOMap", slot);
    }
}

// ===== Animation Support =====
void ShaderLibrary::SetBoneTransforms(Shader *shader, const TEArray<TEMatrix4> &boneTransforms)
{
#ifdef TE_SUPPORT_OPENGL
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL && shader)
    {
        OpenGLShader *glShader = static_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniformMat4Array(glShader, "u_BoneTransforms", boneTransforms);
    }
#endif
}

void ShaderLibrary::SetAnimationTime(Shader *shader, float time)
{
    if (shader)
    {
        shader->SetUniform1f("u_AnimationTime", time);
    }
}

void ShaderLibrary::SetBlendWeights(Shader *shader, const TEVector4 &weights)
{
    if (shader)
    {
        shader->SetUniform4f("u_BlendWeights", weights);
    }
}

// ===== Post-Processing =====
void ShaderLibrary::SetBloom(Shader *shader, float threshold, float intensity)
{
    if (shader)
    {
        shader->SetUniform1f("u_BloomThreshold", threshold);
        shader->SetUniform1f("u_BloomIntensity", intensity);
    }
}

void ShaderLibrary::SetVignette(Shader *shader, float intensity, float radius)
{
    if (shader)
    {
        shader->SetUniform1f("u_VignetteIntensity", intensity);
        shader->SetUniform1f("u_VignetteRadius", radius);
    }
}

void ShaderLibrary::SetChromaticAberration(Shader *shader, float intensity)
{
    if (shader)
    {
        shader->SetUniform1f("u_ChromaticAberration", intensity);
    }
}

void ShaderLibrary::SetMotionBlur(Shader *shader, const TEMatrix4 &previousViewProjection)
{
    if (shader)
    {
        shader->SetUniformMat4("u_PreviousViewProjection", previousViewProjection);
    }
}

void ShaderLibrary::SetDepthOfField(Shader *shader, float focusDistance, float focusRange)
{
    if (shader)
    {
        shader->SetUniform1f("u_FocusDistance", focusDistance);
        shader->SetUniform1f("u_FocusRange", focusRange);
    }
}

// ===== Particle System =====
void ShaderLibrary::SetParticleLife(Shader *shader, float life)
{
    if (shader)
    {
        shader->SetUniform1f("u_ParticleLife", life);
    }
}

void ShaderLibrary::SetParticleSize(Shader *shader, float size)
{
    if (shader)
    {
        shader->SetUniform1f("u_ParticleSize", size);
    }
}

void ShaderLibrary::SetParticleVelocity(Shader *shader, const TEVector &velocity)
{
    if (shader)
    {
        shader->SetUniform3f("u_ParticleVelocity", velocity);
    }
}

void ShaderLibrary::SetParticleAcceleration(Shader *shader, const TEVector &acceleration)
{
    if (shader)
    {
        shader->SetUniform3f("u_ParticleAcceleration", acceleration);
    }
}

void ShaderLibrary::SetParticleColor(Shader *shader, const TEColor &startColor, const TEColor &endColor)
{
    if (shader)
    {
        shader->SetUniform4f("u_ParticleStartColor",
                             TEVector4(startColor.GetR(), startColor.GetG(), startColor.GetB(), startColor.GetA()));
        shader->SetUniform4f("u_ParticleEndColor",
                             TEVector4(endColor.GetR(), endColor.GetG(), endColor.GetB(), endColor.GetA()));
    }
}

// ===== Internal Shader Sources =====
TEString ShaderLibrary::GetBasicVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Normal;
            layout(location = 2) in vec2 a_TexCoord;

            uniform mat4 u_Model;
            uniform mat4 u_View;
            uniform mat4 u_Projection;

            out vec3 v_Position;
            out vec3 v_Normal;
            out vec2 v_TexCoord;

            void main() {
                v_Position = vec3(u_Model * vec4(a_Position, 1.0));
                v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
                v_TexCoord = a_TexCoord;
                gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetBasicFragmentShader()
{
    return R"(
            #version 330 core
            in vec3 v_Position;
            in vec3 v_Normal;
            in vec2 v_TexCoord;

            uniform vec4 u_Color;
            uniform float u_Time;

            out vec4 FragColor;

            void main() {
                vec3 normal = normalize(v_Normal);
                vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
                float diff = max(dot(normal, lightDir), 0.0);
                vec3 diffuse = diff * u_Color.rgb;
                vec3 ambient = 0.3 * u_Color.rgb;
                FragColor = vec4(ambient + diffuse, u_Color.a);
            }
        )";
}

TEString ShaderLibrary::GetTextureVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Normal;
            layout(location = 2) in vec2 a_TexCoord;

            uniform mat4 u_Model;
            uniform mat4 u_View;
            uniform mat4 u_Projection;

            out vec3 v_Position;
            out vec3 v_Normal;
            out vec2 v_TexCoord;

            void main() {
                v_Position = vec3(u_Model * vec4(a_Position, 1.0));
                v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
                v_TexCoord = a_TexCoord;
                gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetTextureFragmentShader()
{
    return R"(
            #version 330 core
            in vec3 v_Position;
            in vec3 v_Normal;
            in vec2 v_TexCoord;

            uniform sampler2D u_Texture;
            uniform vec4 u_Color;
            uniform float u_Time;
            uniform float u_AmbientIntensity;
            uniform vec4 u_AmbientGround;
            uniform vec4 u_AmbientSky;

            out vec4 FragColor;

            void main() {
                vec4 texColor = texture(u_Texture, v_TexCoord) * u_Color;
                if (texColor.a < 0.1) discard;
                
                // Dark baseline if no ambient intensity set to allow additive lights to work
                vec3 ambient = vec3(0.1); 
                if (u_AmbientIntensity > 0.0) {
                     ambient = mix(u_AmbientGround.rgb, u_AmbientSky.rgb, v_TexCoord.y); 
                     ambient *= u_AmbientIntensity;
                }

                FragColor = vec4(texColor.rgb * ambient, texColor.a);
            }
        )";
}

TEString ShaderLibrary::GetColorVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;

            uniform mat4 u_Transform;
            uniform mat4 u_ViewProjection;

            void main() {
                gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetColorFragmentShader()
{
    return R"(
            #version 330 core
            uniform vec4 u_Color;

            out vec4 FragColor;

            void main() {
                FragColor = u_Color;
            }
        )";
}

TEString ShaderLibrary::GetStandardFragmentShader()
{
    return R"(
            #version 330 core
            uniform vec4 u_Color;
            uniform float u_AmbientIntensity;
            uniform vec4 u_AmbientGround;
            uniform vec4 u_AmbientSky;

            out vec4 FragColor;

            void main() {
                // By default, no ambient light means pure black (shadows)
                vec3 ambient = vec3(0.0); 
                if (u_AmbientIntensity > 0.0) {
                     // Since StandardShader has no v_TexCoord, mix using 0.5 ratio
                     ambient = mix(u_AmbientGround.rgb, u_AmbientSky.rgb, 0.5); 
                     ambient *= u_AmbientIntensity;
                }

                FragColor = vec4(u_Color.rgb * ambient, u_Color.a);
            }
        )";
}

TEString ShaderLibrary::GetLightingVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Normal;
            layout(location = 2) in vec2 a_TexCoord;

            uniform mat4 u_Model;
            uniform mat4 u_View;
            uniform mat4 u_Projection;

            out vec3 v_Position;
            out vec3 v_Normal;
            out vec2 v_TexCoord;

            void main() {
                v_Position = vec3(u_Model * vec4(a_Position, 1.0));
                v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
                v_TexCoord = a_TexCoord;
                gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetLightingFragmentShader()
{
    return R"(
            #version 330 core
            in vec3 v_Position;
            in vec3 v_Normal;
            in vec2 v_TexCoord;

            uniform vec3 u_LightPosition;
            uniform vec4 u_LightColor;
            uniform float u_AmbientIntensity;
            uniform float u_DiffuseIntensity;
            uniform float u_SpecularIntensity;
            uniform float u_Shininess;
            uniform vec3 u_CameraPosition;
            uniform sampler2D u_Texture;

            out vec4 FragColor;

            void main() {
                vec3 normal = normalize(v_Normal);
                vec3 lightDir = normalize(u_LightPosition - v_Position);
                vec3 viewDir = normalize(u_CameraPosition - v_Position);
                vec3 reflectDir = reflect(-lightDir, normal);

                float ambient = u_AmbientIntensity;
                float diffuse = max(dot(normal, lightDir), 0.0) * u_DiffuseIntensity;
                float specular = pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess) * u_SpecularIntensity;

                vec4 texColor = texture(u_Texture, v_TexCoord);
                vec3 result = (ambient + diffuse + specular) * u_LightColor.rgb * texColor.rgb;
                FragColor = vec4(result, texColor.a);
            }
        )";
}

TEString ShaderLibrary::GetParticleVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Velocity;
            layout(location = 2) in float a_Life;

            uniform mat4 u_ViewProjection;
            uniform float u_Time;
            uniform float u_ParticleSize;

            out float v_Life;
            out vec3 v_Position;

            void main() {
                v_Life = a_Life;
                v_Position = a_Position + a_Velocity * u_Time;
                gl_Position = u_ViewProjection * vec4(v_Position, 1.0);
                gl_PointSize = u_ParticleSize * (1.0 - a_Life);
            }
        )";
}

TEString ShaderLibrary::GetParticleFragmentShader()
{
    return R"(
            #version 330 core
            in float v_Life;
            in vec3 v_Position;

            uniform vec4 u_ParticleStartColor;
            uniform vec4 u_ParticleEndColor;

            out vec4 FragColor;

            void main() {
                vec4 color = mix(u_ParticleStartColor, u_ParticleEndColor, 1.0 - v_Life);
                FragColor = color;
            }
        )";
}

TEString ShaderLibrary::GetPostProcessVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec2 a_TexCoord;

            out vec2 v_TexCoord;

            void main() {
                v_TexCoord = a_TexCoord;
                gl_Position = vec4(a_Position, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetPostProcessFragmentShader()
{
    return R"(
            #version 330 core
            in vec2 v_TexCoord;

            uniform sampler2D u_ScreenTexture;
            uniform vec2 u_Resolution;
            uniform float u_Time;

            out vec4 FragColor;

            void main() {
                vec4 color = texture(u_ScreenTexture, v_TexCoord);
                FragColor = color;
            }
        )";
}

TEString ShaderLibrary::GetUIVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec2 a_Position;
            layout(location = 1) in vec2 a_TexCoord;

            uniform mat4 u_Projection;

            out vec2 v_TexCoord;

            void main() {
                v_TexCoord = a_TexCoord;
                gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetUIFragmentShader()
{
    return R"(
            #version 330 core
            in vec2 v_TexCoord;

            uniform sampler2D u_Texture;
            uniform vec4 u_Color;

            out vec4 FragColor;

            void main() {
                vec4 texColor = texture(u_Texture, v_TexCoord);
                FragColor = texColor * u_Color;
            }
        )";
}

TEString ShaderLibrary::GetLight2DVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;

            uniform mat4 u_Transform;
            uniform mat4 u_ViewProjection;

            out vec2 v_LocalPos;

            void main() {
                v_LocalPos = a_Position.xy * 2.0;
                gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetLight2DFragmentShader()
{
    return R"(
            #version 330 core
            in vec2 v_LocalPos;

            uniform vec4 u_Color; // Match material SetColor name
            uniform float u_Intensity;
            uniform int u_LightType; // 0=Point, 1=Spot, 2=Line

            uniform vec2 u_Direction;
            uniform float u_InnerAngle;
            uniform float u_OuterAngle;
            uniform float u_LineLength;
            uniform float u_Radius;
            uniform float u_FalloffExponent;

            out vec4 FragColor;

            void main() {
                float falloff = 0.0;
                
                if (u_LightType == 0 || u_LightType == 1) { // Point & Spot
                    float dist = length(v_LocalPos);
                    if (dist > 1.0) discard;
                    
                    // Base falloff using configurable exponent (default 2.0)
                    float baseFalloff = pow(1.0 - dist, u_FalloffExponent);
                    
                    // High-intensity central hotspot for that "bright core" look
                    // Falloff is very sharp (exp -10) to keep it at the center
                    float core = exp(-dist * 10.0) * 2.0;
                    
                    // Combine and multiply by linear intensity
                    // NO CLAMPing to 1.0 here, to allow the center to blow out/saturated
                    falloff = (baseFalloff + core) * u_Intensity;

                    if (u_LightType == 1) { // Spot
                        float radInner = radians(u_InnerAngle);
                        float radOuter = radians(u_OuterAngle);
                        float cosTheta = dot(normalize(v_LocalPos), normalize(u_Direction));
                        float cosInner = cos(radInner);
                        float cosOuter = cos(radOuter);
                        
                        float spotEffect = smoothstep(cosOuter, cosInner, cosTheta);
                        falloff *= spotEffect;
                    }
                } else if (u_LightType == 2) { // Line
                    float halfLen = (u_LineLength * 0.5);
                    float r = u_Radius;
                    
                    float quadX = u_LineLength + r * 2.0;
                    float quadY = r * 2.0;
                    vec2 p = v_LocalPos * vec2(quadX * 0.5, quadY * 0.5);
                    
                    float d = length(p - vec2(clamp(p.x, -halfLen, halfLen), 0.0));
                    float dist = d / r;
                    if (dist > 1.0) discard;
                    
                    float baseFalloff = pow(1.0 - dist, u_FalloffExponent);
                    float core = exp(-dist * 12.0) * 1.5;
                    falloff = (baseFalloff + core) * u_Intensity;
                }

                FragColor = vec4(u_Color.rgb * falloff, falloff);
            }
        )";
}

TEString ShaderLibrary::GetAmbientGradientFragmentShader()
{
    return R"(
            #version 330 core
            in vec2 v_LocalPos; // -1 to 1

            uniform vec4 u_SkyColor;
            uniform vec4 u_HorizonColor;
            uniform vec4 u_GroundColor;
            uniform float u_Intensity;
            uniform float u_HorizonHeight;
            uniform float u_HorizonSpread;

            out vec4 FragColor;

            void main() {
                // v_LocalPos.y is in world space? No, NDC-ish from full screen quad.
                // Assuming it covers -1 to 1 NDC. y goes from 0 (bottom) to 1 (top).
                float y = v_LocalPos.y * 0.5 + 0.5;
                
                vec3 color;
                if (y > u_HorizonHeight) {
                    float t = clamp((y - u_HorizonHeight) / u_HorizonSpread, 0.0, 1.0);
                    color = mix(u_HorizonColor.rgb, u_SkyColor.rgb, t);
                } else {
                    float t = clamp((u_HorizonHeight - y) / u_HorizonSpread, 0.0, 1.0);
                    color = mix(u_HorizonColor.rgb, u_GroundColor.rgb, t);
                }
                
                FragColor = vec4(color * u_Intensity, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetLightBlendVertexShader()
{
    return R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;

            uniform mat4 u_Transform;
            uniform mat4 u_ViewProjection;

            out vec2 v_TexCoord;

            void main() {
                v_TexCoord = a_Position.xy + 0.5;
                gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
            }
        )";
}

TEString ShaderLibrary::GetLightBlendFragmentShader()
{
    return R"(
            #version 330 core
            in vec2 v_TexCoord;
            uniform sampler2D u_Texture;

            out vec4 FragColor;

            void main() {
                FragColor = texture(u_Texture, v_TexCoord);
            }
        )";
}
