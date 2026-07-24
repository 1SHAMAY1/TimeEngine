#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/PreRequisites.h"
#include "Renderer/Shader.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include <glm/glm.hpp>
#include <unordered_map>

namespace TE
{

enum class MaterialPassNodeType
{
    BaseSurfaceSlab = 0,
    CoatNormalSlab,
    EmissiveGlowSlab,
    UVScrollerModifier,
    BlendOutputState,
    CustomUniformOverride,
    PBRMetallicRoughnessSlab,
    FresnelRimLightModifier,
    ColorGradingModifier,
    SubsurfaceScatteringSlab,
    DissolveMaskModifier,
    TriplanarMappingSlab,
    ParallaxOcclusionSlab,
    GradientRampColorizerModifier,
    ChromaticAberrationModifier,
    SpriteSheetFrameSamplerSlab
};

struct MaterialPassNode
{
    std::string Name;
    MaterialPassNodeType Type = MaterialPassNodeType::BaseSurfaceSlab;
    bool Enabled = true;
    int TargetQueueIndex = 0;
    std::string QueueName = "Queue 1";

    std::string TexturePath;
    std::shared_ptr<class Texture> TextureRef;
    TEVector4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float FloatVal1 = 1.0f; // e.g. Roughness / Bump Depth / Intensity / Speed X
    float FloatVal2 = 0.0f; // e.g. Metallic / Speed Y
    float FloatVal3 = 1.0f; // e.g. Tiling X
    float FloatVal4 = 1.0f; // e.g. Tiling Y
    int BlendMode = 1;      // 0 = Opaque, 1 = AlphaBlend, 2 = Additive, 3 = Multiply
};

class TE_API Material : public Asset
{
public:
    Material(const std::shared_ptr<Shader> &shader);
    Material();
    virtual ~Material();

    void SetColor(const TEColor &color);
    const TEColor &GetColor() const;

    void SetUniform(const std::string &name, float value);
    void SetUniform(const std::string &name, int value);
    void SetUniform(const std::string &name, const glm::vec2 &value);
    void SetUniform(const std::string &name, const glm::vec3 &value);
    void SetUniform(const std::string &name, const glm::vec4 &value);
    void SetUniform(const std::string &name, const glm::mat4 &value);

    void SetShader(const std::shared_ptr<Shader> &shader);
    std::shared_ptr<Shader> GetShader() const;

    // Set all uniforms (for now, just color and custom ones)
    void ApplyUniforms();

    // Pass Stack Methods
    const std::vector<MaterialPassNode> &GetPassStack() const { return m_PassStack; }
    std::vector<MaterialPassNode> &GetPassStack() { return m_PassStack; }
    void AddPassNode(const MaterialPassNode &node) { m_PassStack.push_back(node); }
    void RemovePassNode(size_t index)
    {
        if (index < m_PassStack.size())
            m_PassStack.erase(m_PassStack.begin() + index);
    }

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string &GetType() const override
    {
        static std::string type = "Material";
        return type;
    }
    virtual const std::string &GetName() const override { return m_Name; }
    virtual const std::string &GetHoverDescription() const override
    {
        static std::string desc = "Material Asset";
        return desc;
    }
    virtual std::string GetDefaultExtension() const override { return ".tematerial"; }
    virtual std::string GetDefaultIconPath() const override { return "Resources/Editor/MaterialIcon.png"; }

    virtual std::shared_ptr<class Texture> GetIcon() const override { return nullptr; }
    virtual std::shared_ptr<class Texture> GetThumbnail() const override { return nullptr; }

    virtual void OnContentBrowserCreate(const std::filesystem::path &path) override;

    void SetName(const std::string &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

private:
    std::shared_ptr<Shader> m_Shader;
    TEColor m_Color;

    std::unordered_map<std::string, float> m_FloatUniforms;
    std::unordered_map<std::string, int> m_IntUniforms;
    std::unordered_map<std::string, glm::vec2> m_Vec2Uniforms;
    std::unordered_map<std::string, glm::vec3> m_Vec3Uniforms;
    std::unordered_map<std::string, glm::vec4> m_Vec4Uniforms;
    std::unordered_map<std::string, glm::mat4> m_Mat4Uniforms;

    std::vector<MaterialPassNode> m_PassStack;

    AssetHandle m_Handle = 0;
    std::string m_Name = "Unnamed Material";
};

} // namespace TE