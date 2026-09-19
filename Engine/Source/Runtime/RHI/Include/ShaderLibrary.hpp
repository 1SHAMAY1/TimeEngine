#pragma once
#include "PreRequisites.h"
#include "GameplayUtils.hpp"
#include "Shader.hpp"
#include "TEColor.hpp"
#include "MathUtils.hpp"

class ShaderLibrary
{
public:
    // ===== Shader Creation =====
    static TERef<Shader> CreateBasicShader();
    static TERef<Shader> CreateTextureShader();
    static TERef<Shader> CreateColorShader();
    static TERef<Shader> CreateStandardShader();
    static TERef<Shader> CreateLightingShader();
    static TERef<Shader> CreateParticleShader();
    static TERef<Shader> CreatePostProcessShader();
    static TERef<Shader> CreateUIShader();
    static TERef<Shader> CreateLight2DShader();
    static TERef<Shader> CreateAmbientGradientShader();
    static TERef<Shader> CreateLightBlendShader();

    // ===== Common Shader Functions =====
    static void SetMVP(Shader *shader, const TEMatrix4 &model, const TEMatrix4 &view, const TEMatrix4 &projection);
    static void SetColor(Shader *shader, const TEColor &color);
    static void SetColor(Shader *shader, const TEVector4 &color);
    static void SetTransform(Shader *shader, const TEMatrix4 &transform);
    static void SetViewProjection(Shader *shader, const TEMatrix4 &viewProjection);
    static void SetLightPosition(Shader *shader, const TEVector &position);
    static void SetLightColor(Shader *shader, const TEColor &color);
    static void SetAmbientLight(Shader *shader, float intensity);
    static void SetDiffuseLight(Shader *shader, float intensity);
    static void SetSpecularLight(Shader *shader, float intensity, float shininess);
    static void SetTexture(Shader *shader, int slot);
    static void SetTime(Shader *shader, float time);
    static void SetResolution(Shader *shader, const TEVector2 &resolution);
    static void SetCameraPosition(Shader *shader, const TEVector &position);
    static void SetFog(Shader *shader, const TEColor &color, float density, float start, float end);

    // ===== Utility Functions =====
    static TEMatrix4 CreateModelMatrix(const TEVector &position, const TEVector &rotation, const TEVector &scale);
    static TEMatrix4 CreateViewMatrix(const TEVector &position, const TEVector &target,
                                      const TEVector &up = TEVector(0, 1, 0));
    static TEMatrix4 CreateProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane);
    static TEMatrix4 CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane,
                                              float farPlane);

    // ===== Color Utilities =====
    static void SetTint(Shader *shader, const TEColor &tint, float intensity = 1.0f);
    static void SetBrightness(Shader *shader, float brightness);
    static void SetContrast(Shader *shader, float contrast);
    static void SetSaturation(Shader *shader, float saturation);
    static void SetGamma(Shader *shader, float gamma);

    // ===== Material Properties =====
    static void SetMaterial(Shader *shader, const TEColor &ambient, const TEColor &diffuse, const TEColor &specular,
                            float shininess);
    static void SetEmissive(Shader *shader, const TEColor &emissive);
    static void SetMetallic(Shader *shader, float metallic);
    static void SetRoughness(Shader *shader, float roughness);
    static void SetNormalMap(Shader *shader, int slot);
    static void SetRoughnessMap(Shader *shader, int slot);
    static void SetMetallicMap(Shader *shader, int slot);
    static void SetAOMap(Shader *shader, int slot);

    // ===== Animation Support =====
    static void SetBoneTransforms(Shader *shader, const TEArray<TEMatrix4> &boneTransforms);
    static void SetAnimationTime(Shader *shader, float time);
    static void SetBlendWeights(Shader *shader, const TEVector4 &weights);

    // ===== Post-Processing =====
    static void SetBloom(Shader *shader, float threshold, float intensity);
    static void SetVignette(Shader *shader, float intensity, float radius);
    static void SetChromaticAberration(Shader *shader, float intensity);
    static void SetMotionBlur(Shader *shader, const TEMatrix4 &previousViewProjection);
    static void SetDepthOfField(Shader *shader, float focusDistance, float focusRange);

    // ===== Particle System =====
    static void SetParticleLife(Shader *shader, float life);
    static void SetParticleSize(Shader *shader, float size);
    static void SetParticleVelocity(Shader *shader, const TEVector &velocity);
    static void SetParticleAcceleration(Shader *shader, const TEVector &acceleration);
    static void SetParticleColor(Shader *shader, const TEColor &startColor, const TEColor &endColor);

protected:
    static TEMap<TEString, TERef<Shader>> s_ShaderCache;

    // ===== Internal Shader Sources =====
    static TEString GetBasicVertexShader();
    static TEString GetBasicFragmentShader();
    static TEString GetTextureVertexShader();
    static TEString GetTextureFragmentShader();
    static TEString GetColorVertexShader();
    static TEString GetColorFragmentShader();
    static TEString GetStandardFragmentShader();
    static TEString GetLightingVertexShader();
    static TEString GetLightingFragmentShader();
    static TEString GetParticleVertexShader();
    static TEString GetParticleFragmentShader();
    static TEString GetPostProcessVertexShader();
    static TEString GetPostProcessFragmentShader();
    static TEString GetUIVertexShader();
    static TEString GetUIFragmentShader();
    static TEString GetLight2DVertexShader();
    static TEString GetLight2DFragmentShader();
    static TEString GetAmbientGradientFragmentShader();
    static TEString GetLightBlendVertexShader();
    static TEString GetLightBlendFragmentShader();
};
