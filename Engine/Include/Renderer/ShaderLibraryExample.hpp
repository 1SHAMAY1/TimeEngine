#pragma once
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/OpenGL/OpenGLShaderLibrary.hpp"
#include "Layers/ProfilingLayer.hpp"
#include "Renderer/TEColor.hpp"
#include <memory>

namespace TE {

    class ShaderLibraryExample {
    public:
        static void DemonstrateShaderLibrary();
        static void DemonstrateProfiling();
        static void DemonstrateOpenGLFeatures();

    private:
        static void SetupBasicRendering();
        static void SetupLightingRendering();
        static void SetupParticleRendering();
        static void SetupPostProcessing();
        static void SetupUIRendering();
    };

    // ===== Example Usage Functions =====
    
    // Example 1: Basic Color Shader
    inline void ExampleBasicColorShader() {
        auto shader = ShaderLibrary::CreateColorShader();
        shader->Bind();
        
        // Set color using TEColor
        TEColor redColor = TEColor::Red();
        ShaderLibrary::SetColor(shader.get(), redColor);
        
        // Set transform
        glm::mat4 transform = ShaderLibrary::CreateModelMatrix(
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f)
        );
        ShaderLibrary::SetTransform(shader.get(), transform);
        
        // Set view projection
        glm::mat4 view = ShaderLibrary::CreateViewMatrix(
            glm::vec3(0.0f, 0.0f, 5.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
        );
        glm::mat4 projection = ShaderLibrary::CreateProjectionMatrix(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
        ShaderLibrary::SetViewProjection(shader.get(), projection * view);
    }

    // Example 2: Texture Shader with Lighting
    inline void ExampleTextureLightingShader() {
        auto shader = ShaderLibrary::CreateLightingShader();
        shader->Bind();
        
        // Set material properties
        TEColor ambient = TEColor(0.2f, 0.2f, 0.2f, 1.0f);
        TEColor diffuse = TEColor(0.8f, 0.8f, 0.8f, 1.0f);
        TEColor specular = TEColor(1.0f, 1.0f, 1.0f, 1.0f);
        ShaderLibrary::SetMaterial(shader.get(), ambient, diffuse, specular, 32.0f);
        
        // Set light properties
        ShaderLibrary::SetLightPosition(shader.get(), glm::vec3(5.0f, 5.0f, 5.0f));
        ShaderLibrary::SetLightColor(shader.get(), TEColor::White());
        ShaderLibrary::SetAmbientLight(shader.get(), 0.3f);
        ShaderLibrary::SetDiffuseLight(shader.get(), 0.7f);
        ShaderLibrary::SetSpecularLight(shader.get(), 1.0f, 32.0f);
        
        // Set camera position
        ShaderLibrary::SetCameraPosition(shader.get(), glm::vec3(0.0f, 0.0f, 5.0f));
        
        // Set texture
        ShaderLibrary::SetTexture(shader.get(), 0);
    }

    // Example 3: Particle Shader
    inline void ExampleParticleShader() {
        auto shader = ShaderLibrary::CreateParticleShader();
        shader->Bind();
        
        // Set particle properties
        ShaderLibrary::SetParticleLife(shader.get(), 0.5f);
        ShaderLibrary::SetParticleSize(shader.get(), 10.0f);
        ShaderLibrary::SetParticleVelocity(shader.get(), glm::vec3(0.0f, 1.0f, 0.0f));
        ShaderLibrary::SetParticleAcceleration(shader.get(), glm::vec3(0.0f, -9.8f, 0.0f));
        
        // Set particle colors
        TEColor startColor = TEColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
        TEColor endColor = TEColor(1.0f, 0.0f, 0.0f, 0.0f);   // Red with alpha 0
        ShaderLibrary::SetParticleColor(shader.get(), startColor, endColor);
        
        // Set time for animation
        ShaderLibrary::SetTime(shader.get(), 0.0f);
    }

    // Example 4: Post-Processing Shader
    inline void ExamplePostProcessShader() {
        auto shader = ShaderLibrary::CreatePostProcessShader();
        shader->Bind();
        
        // Set post-processing effects
        ShaderLibrary::SetBloom(shader.get(), 0.8f, 1.2f);
        ShaderLibrary::SetVignette(shader.get(), 0.3f, 0.5f);
        ShaderLibrary::SetChromaticAberration(shader.get(), 0.01f);
        
        // Set color adjustments
        ShaderLibrary::SetBrightness(shader.get(), 1.1f);
        ShaderLibrary::SetContrast(shader.get(), 1.2f);
        ShaderLibrary::SetSaturation(shader.get(), 1.1f);
        ShaderLibrary::SetGamma(shader.get(), 2.2f);
        
        // Set resolution
        ShaderLibrary::SetResolution(shader.get(), glm::vec2(1920.0f, 1080.0f));
    }

    // Example 5: UI Shader
    inline void ExampleUIShader() {
        auto shader = ShaderLibrary::CreateUIShader();
        shader->Bind();
        
        // Set UI color with tint
        TEColor uiColor = TEColor(0.2f, 0.6f, 1.0f, 0.8f); // Blue with transparency
        ShaderLibrary::SetColor(shader.get(), uiColor);
        ShaderLibrary::SetTint(shader.get(), TEColor::White(), 0.5f);
        
        // Set orthographic projection for UI
        glm::mat4 ortho = ShaderLibrary::CreateOrthographicMatrix(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        ShaderLibrary::SetViewProjection(shader.get(), ortho);
    }

    // Example 6: OpenGL-Specific Features
    inline void ExampleOpenGLFeatures() {
        // Enable OpenGL features
        OpenGLShaderLibrary::EnableBlending();
        OpenGLShaderLibrary::SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        OpenGLShaderLibrary::EnableDepthTest();
        OpenGLShaderLibrary::SetDepthFunc(GL_LESS);
        OpenGLShaderLibrary::EnableCullFace();
        OpenGLShaderLibrary::SetCullFace(GL_BACK);
        OpenGLShaderLibrary::SetFrontFace(GL_CCW);
        
        // Create and use a compute shader
        std::string computeSource = R"(
            #version 430
            layout(local_size_x = 256) in;
            
            layout(std430, binding = 0) buffer ParticleBuffer {
                vec4 particles[];
            };
            
            uniform float u_Time;
            uniform float u_DeltaTime;
            
            void main() {
                uint index = gl_GlobalInvocationID.x;
                vec4 particle = particles[index];
                
                // Update particle position
                particle.xyz += particle.xyz * u_DeltaTime;
                
                // Update particle life
                particle.w -= u_DeltaTime;
                
                particles[index] = particle;
            }
        )";
        
        auto computeShader = OpenGLShaderLibrary::CreateOpenGLComputeShader(computeSource);
        if (computeShader) {
            computeShader->Bind();
            OpenGLShaderLibrary::SetUniform1f(computeShader.get(), "u_Time", 0.0f);
            OpenGLShaderLibrary::SetUniform1f(computeShader.get(), "u_DeltaTime", 0.016f);
            
            // Dispatch compute shader
            OpenGLShaderLibrary::DispatchCompute(computeShader->GetRendererID(), 1000, 1, 1);
            OpenGLShaderLibrary::MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }

    // Example 7: Profiling Integration
    inline void ExampleProfilingIntegration() {
        // Create profiling layer
        auto profilingLayer = std::make_unique<ProfilingLayer>();
        profilingLayer->OnAttach();
        
        // Record rendering statistics
        profilingLayer->RecordDrawCall();
        profilingLayer->RecordTriangle(1000);
        profilingLayer->RecordVertex(3000);
        profilingLayer->RecordTexture(5);
        profilingLayer->RecordShader(3);
        
        // Update profiling
        profilingLayer->OnUpdate();
        profilingLayer->OnImGuiRender();
        
        // Get performance metrics
        const auto& metrics = profilingLayer->GetCurrentMetrics();
        float fps = metrics.fps;
        float cpuUsage = metrics.cpuUsage;
        float ramUsage = metrics.ramUsage;
        
        // Use metrics for optimization
        if (fps < 30.0f) {
            // Reduce quality settings
            ShaderLibrary::SetResolution(nullptr, glm::vec2(1280.0f, 720.0f));
        }
        
        if (cpuUsage > 80.0f) {
            // Reduce CPU-intensive operations
        }
        
        if (ramUsage > 90.0f) {
            // Free unused resources
        }
    }

    // Example 8: Advanced Shader Features
    inline void ExampleAdvancedShaderFeatures() {
        auto shader = ShaderLibrary::CreateLightingShader();
        shader->Bind();
        
        // Set PBR material properties
        ShaderLibrary::SetMetallic(shader.get(), 0.8f);
        ShaderLibrary::SetRoughness(shader.get(), 0.2f);
        ShaderLibrary::SetEmissive(shader.get(), TEColor(0.1f, 0.0f, 0.0f, 1.0f));
        
        // Set texture maps
        ShaderLibrary::SetNormalMap(shader.get(), 1);
        ShaderLibrary::SetRoughnessMap(shader.get(), 2);
        ShaderLibrary::SetMetallicMap(shader.get(), 3);
        ShaderLibrary::SetAOMap(shader.get(), 4);
        
        // Set fog
        ShaderLibrary::SetFog(shader.get(), TEColor(0.5f, 0.5f, 0.5f, 1.0f), 0.01f, 10.0f, 100.0f);
        
        // Set animation data
        std::vector<glm::mat4> boneTransforms(100, glm::mat4(1.0f));
        ShaderLibrary::SetBoneTransforms(shader.get(), boneTransforms);
        ShaderLibrary::SetAnimationTime(shader.get(), 0.0f);
        ShaderLibrary::SetBlendWeights(shader.get(), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    }

} 