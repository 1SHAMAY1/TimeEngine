#pragma once
#include "Core/PreRequisites.h"
#include "Renderer/TEColor.hpp"
#include "Renderer/Shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <unordered_map>

namespace TE {

    class ShaderLibrary {
    public:
        // ===== Shader Creation =====
        static std::shared_ptr<Shader> CreateBasicShader();
        static std::shared_ptr<Shader> CreateTextureShader();
        static std::shared_ptr<Shader> CreateColorShader();
        static std::shared_ptr<Shader> CreateLightingShader();
        static std::shared_ptr<Shader> CreateParticleShader();
        static std::shared_ptr<Shader> CreatePostProcessShader();
        static std::shared_ptr<Shader> CreateUIShader();

        // ===== Common Shader Functions =====
        static void SetMVP(Shader* shader, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
        static void SetColor(Shader* shader, const TEColor& color);
        static void SetColor(Shader* shader, const glm::vec4& color);
        static void SetTransform(Shader* shader, const glm::mat4& transform);
        static void SetViewProjection(Shader* shader, const glm::mat4& viewProjection);
        static void SetLightPosition(Shader* shader, const glm::vec3& position);
        static void SetLightColor(Shader* shader, const TEColor& color);
        static void SetAmbientLight(Shader* shader, float intensity);
        static void SetDiffuseLight(Shader* shader, float intensity);
        static void SetSpecularLight(Shader* shader, float intensity, float shininess);
        static void SetTexture(Shader* shader, int slot);
        static void SetTime(Shader* shader, float time);
        static void SetResolution(Shader* shader, const glm::vec2& resolution);
        static void SetCameraPosition(Shader* shader, const glm::vec3& position);
        static void SetFog(Shader* shader, const TEColor& color, float density, float start, float end);

        // ===== Utility Functions =====
        static glm::mat4 CreateModelMatrix(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
        static glm::mat4 CreateViewMatrix(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
        static glm::mat4 CreateProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane);
        static glm::mat4 CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane);

        // ===== Color Utilities =====
        static void SetTint(Shader* shader, const TEColor& tint, float intensity = 1.0f);
        static void SetBrightness(Shader* shader, float brightness);
        static void SetContrast(Shader* shader, float contrast);
        static void SetSaturation(Shader* shader, float saturation);
        static void SetGamma(Shader* shader, float gamma);

        // ===== Material Properties =====
        static void SetMaterial(Shader* shader, const TEColor& ambient, const TEColor& diffuse, const TEColor& specular, float shininess);
        static void SetEmissive(Shader* shader, const TEColor& emissive);
        static void SetMetallic(Shader* shader, float metallic);
        static void SetRoughness(Shader* shader, float roughness);
        static void SetNormalMap(Shader* shader, int slot);
        static void SetRoughnessMap(Shader* shader, int slot);
        static void SetMetallicMap(Shader* shader, int slot);
        static void SetAOMap(Shader* shader, int slot);

        // ===== Animation Support =====
        static void SetBoneTransforms(Shader* shader, const std::vector<glm::mat4>& boneTransforms);
        static void SetAnimationTime(Shader* shader, float time);
        static void SetBlendWeights(Shader* shader, const glm::vec4& weights);

        // ===== Post-Processing =====
        static void SetBloom(Shader* shader, float threshold, float intensity);
        static void SetVignette(Shader* shader, float intensity, float radius);
        static void SetChromaticAberration(Shader* shader, float intensity);
        static void SetMotionBlur(Shader* shader, const glm::mat4& previousViewProjection);
        static void SetDepthOfField(Shader* shader, float focusDistance, float focusRange);

        // ===== Particle System =====
        static void SetParticleLife(Shader* shader, float life);
        static void SetParticleSize(Shader* shader, float size);
        static void SetParticleVelocity(Shader* shader, const glm::vec3& velocity);
        static void SetParticleAcceleration(Shader* shader, const glm::vec3& acceleration);
        static void SetParticleColor(Shader* shader, const TEColor& startColor, const TEColor& endColor);

    protected:
        static std::unordered_map<std::string, std::shared_ptr<Shader>> s_ShaderCache;
        
        // ===== Internal Shader Sources =====
        static std::string GetBasicVertexShader();
        static std::string GetBasicFragmentShader();
        static std::string GetTextureVertexShader();
        static std::string GetTextureFragmentShader();
        static std::string GetColorVertexShader();
        static std::string GetColorFragmentShader();
        static std::string GetLightingVertexShader();
        static std::string GetLightingFragmentShader();
        static std::string GetParticleVertexShader();
        static std::string GetParticleFragmentShader();
        static std::string GetPostProcessVertexShader();
        static std::string GetPostProcessFragmentShader();
        static std::string GetUIVertexShader();
        static std::string GetUIFragmentShader();
    };

} 