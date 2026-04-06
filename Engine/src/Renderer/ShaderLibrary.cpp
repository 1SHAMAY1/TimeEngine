#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLShader.hpp"
#include "Renderer/OpenGL/OpenGLShaderLibrary.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TE
{

std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderLibrary::s_ShaderCache;

// ===== Shader Creation =====
std::shared_ptr<Shader> ShaderLibrary::CreateBasicShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLBasicShader();
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderLibrary::CreateTextureShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLTextureShader();
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderLibrary::CreateColorShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLColorShader();
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderLibrary::CreateStandardShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLStandardShader();
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderLibrary::CreateLightingShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLLightingShader();
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderLibrary::CreateLight2DShader()
{
    return std::shared_ptr<Shader>(Shader::Create(GetLight2DVertexShader(), GetLight2DFragmentShader()));
}

std::shared_ptr<Shader> ShaderLibrary::CreateAmbientGradientShader()
{
    return std::shared_ptr<Shader>(Shader::Create(GetLight2DVertexShader(), GetAmbientGradientFragmentShader()));
}

std::shared_ptr<Shader> ShaderLibrary::CreateLightBlendShader()
{
    return std::shared_ptr<Shader>(Shader::Create(GetLightBlendVertexShader(), GetLightBlendFragmentShader()));
}

std::shared_ptr<Shader> ShaderLibrary::CreateParticleShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLParticleShader();
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderLibrary::CreatePostProcessShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLPostProcessShader();
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderLibrary::CreateUIShader()
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        return OpenGLShaderLibrary::CreateOpenGLUIShader();
    }
    return nullptr;
}

// ===== Common Shader Functions =====
void ShaderLibrary::SetMVP(Shader *shader, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniformMat4(glShader, "u_Model", model);
        OpenGLShaderLibrary::SetUniformMat4(glShader, "u_View", view);
        OpenGLShaderLibrary::SetUniformMat4(glShader, "u_Projection", projection);
        OpenGLShaderLibrary::SetUniformMat4(glShader, "u_MVP", projection * view * model);
    }
}

void ShaderLibrary::SetColor(Shader *shader, const TEColor &color) { SetColor(shader, color.GetValue()); }

void ShaderLibrary::SetColor(Shader *shader, const glm::vec4 &color)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_Color", color);
    }
}

void ShaderLibrary::SetTransform(Shader *shader, const glm::mat4 &transform)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniformMat4(glShader, "u_Transform", transform);
    }
}

void ShaderLibrary::SetViewProjection(Shader *shader, const glm::mat4 &viewProjection)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniformMat4(glShader, "u_ViewProjection", viewProjection);
    }
}

void ShaderLibrary::SetLightPosition(Shader *shader, const glm::vec3 &position)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform3f(glShader, "u_LightPosition", position);
    }
}

void ShaderLibrary::SetLightColor(Shader *shader, const TEColor &color)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_LightColor", color.GetValue());
    }
}

void ShaderLibrary::SetAmbientLight(Shader *shader, float intensity)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_AmbientIntensity", intensity);
    }
}

void ShaderLibrary::SetDiffuseLight(Shader *shader, float intensity)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_DiffuseIntensity", intensity);
    }
}

void ShaderLibrary::SetSpecularLight(Shader *shader, float intensity, float shininess)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_SpecularIntensity", intensity);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Shininess", shininess);
    }
}

void ShaderLibrary::SetTexture(Shader *shader, int slot)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1i(glShader, "u_Texture", slot);
    }
}

void ShaderLibrary::SetTime(Shader *shader, float time)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Time", time);
    }
}

void ShaderLibrary::SetResolution(Shader *shader, const glm::vec2 &resolution)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform2f(glShader, "u_Resolution", resolution);
    }
}

void ShaderLibrary::SetCameraPosition(Shader *shader, const glm::vec3 &position)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform3f(glShader, "u_CameraPosition", position);
    }
}

void ShaderLibrary::SetFog(Shader *shader, const TEColor &color, float density, float start, float end)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_FogColor", color.GetValue());
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_FogDensity", density);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_FogStart", start);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_FogEnd", end);
    }
}

// ===== Utility Functions =====
glm::mat4 ShaderLibrary::CreateModelMatrix(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    return model;
}

glm::mat4 ShaderLibrary::CreateViewMatrix(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
    return glm::lookAt(position, target, up);
}

glm::mat4 ShaderLibrary::CreateProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

glm::mat4 ShaderLibrary::CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane,
                                                  float farPlane)
{
    return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
}

// ===== Color Utilities =====
void ShaderLibrary::SetTint(Shader *shader, const TEColor &tint, float intensity)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_Tint", tint.GetValue());
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_TintIntensity", intensity);
    }
}

void ShaderLibrary::SetBrightness(Shader *shader, float brightness)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Brightness", brightness);
    }
}

void ShaderLibrary::SetContrast(Shader *shader, float contrast)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Contrast", contrast);
    }
}

void ShaderLibrary::SetSaturation(Shader *shader, float saturation)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Saturation", saturation);
    }
}

void ShaderLibrary::SetGamma(Shader *shader, float gamma)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Gamma", gamma);
    }
}

// ===== Material Properties =====
void ShaderLibrary::SetMaterial(Shader *shader, const TEColor &ambient, const TEColor &diffuse, const TEColor &specular,
                                float shininess)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_Material.ambient", ambient.GetValue());
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_Material.diffuse", diffuse.GetValue());
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_Material.specular", specular.GetValue());
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Material.shininess", shininess);
    }
}

void ShaderLibrary::SetEmissive(Shader *shader, const TEColor &emissive)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_Emissive", emissive.GetValue());
    }
}

void ShaderLibrary::SetMetallic(Shader *shader, float metallic)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Metallic", metallic);
    }
}

void ShaderLibrary::SetRoughness(Shader *shader, float roughness)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_Roughness", roughness);
    }
}

void ShaderLibrary::SetNormalMap(Shader *shader, int slot)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1i(glShader, "u_NormalMap", slot);
    }
}

void ShaderLibrary::SetRoughnessMap(Shader *shader, int slot)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1i(glShader, "u_RoughnessMap", slot);
    }
}

void ShaderLibrary::SetMetallicMap(Shader *shader, int slot)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1i(glShader, "u_MetallicMap", slot);
    }
}

void ShaderLibrary::SetAOMap(Shader *shader, int slot)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1i(glShader, "u_AOMap", slot);
    }
}

// ===== Animation Support =====
void ShaderLibrary::SetBoneTransforms(Shader *shader, const std::vector<glm::mat4> &boneTransforms)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniformMat4Array(glShader, "u_BoneTransforms", boneTransforms);
    }
}

void ShaderLibrary::SetAnimationTime(Shader *shader, float time)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_AnimationTime", time);
    }
}

void ShaderLibrary::SetBlendWeights(Shader *shader, const glm::vec4 &weights)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_BlendWeights", weights);
    }
}

// ===== Post-Processing =====
void ShaderLibrary::SetBloom(Shader *shader, float threshold, float intensity)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_BloomThreshold", threshold);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_BloomIntensity", intensity);
    }
}

void ShaderLibrary::SetVignette(Shader *shader, float intensity, float radius)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_VignetteIntensity", intensity);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_VignetteRadius", radius);
    }
}

void ShaderLibrary::SetChromaticAberration(Shader *shader, float intensity)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_ChromaticAberration", intensity);
    }
}

void ShaderLibrary::SetMotionBlur(Shader *shader, const glm::mat4 &previousViewProjection)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniformMat4(glShader, "u_PreviousViewProjection", previousViewProjection);
    }
}

void ShaderLibrary::SetDepthOfField(Shader *shader, float focusDistance, float focusRange)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_FocusDistance", focusDistance);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_FocusRange", focusRange);
    }
}

// ===== Particle System =====
void ShaderLibrary::SetParticleLife(Shader *shader, float life)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_ParticleLife", life);
    }
}

void ShaderLibrary::SetParticleSize(Shader *shader, float size)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform1f(glShader, "u_ParticleSize", size);
    }
}

void ShaderLibrary::SetParticleVelocity(Shader *shader, const glm::vec3 &velocity)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform3f(glShader, "u_ParticleVelocity", velocity);
    }
}

void ShaderLibrary::SetParticleAcceleration(Shader *shader, const glm::vec3 &acceleration)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform3f(glShader, "u_ParticleAcceleration", acceleration);
    }
}

void ShaderLibrary::SetParticleColor(Shader *shader, const TEColor &startColor, const TEColor &endColor)
{
    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        OpenGLShader *glShader = dynamic_cast<OpenGLShader *>(shader);
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_ParticleStartColor", startColor.GetValue());
        OpenGLShaderLibrary::SetUniform4f(glShader, "u_ParticleEndColor", endColor.GetValue());
    }
}

// ===== Internal Shader Sources =====
std::string ShaderLibrary::GetBasicVertexShader()
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

std::string ShaderLibrary::GetBasicFragmentShader()
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

std::string ShaderLibrary::GetTextureVertexShader()
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

std::string ShaderLibrary::GetTextureFragmentShader()
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

std::string ShaderLibrary::GetColorVertexShader()
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

std::string ShaderLibrary::GetColorFragmentShader()
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

std::string ShaderLibrary::GetStandardFragmentShader()
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

std::string ShaderLibrary::GetLightingVertexShader()
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

std::string ShaderLibrary::GetLightingFragmentShader()
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

std::string ShaderLibrary::GetParticleVertexShader()
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

std::string ShaderLibrary::GetParticleFragmentShader()
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

std::string ShaderLibrary::GetPostProcessVertexShader()
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

std::string ShaderLibrary::GetPostProcessFragmentShader()
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

std::string ShaderLibrary::GetUIVertexShader()
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

std::string ShaderLibrary::GetUIFragmentShader()
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

std::string ShaderLibrary::GetLight2DVertexShader()
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

std::string ShaderLibrary::GetLight2DFragmentShader()
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

std::string ShaderLibrary::GetAmbientGradientFragmentShader()
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

std::string ShaderLibrary::GetLightBlendVertexShader()
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

std::string ShaderLibrary::GetLightBlendFragmentShader()
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

} // namespace TE