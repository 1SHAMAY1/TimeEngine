#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include <glm/glm.hpp>


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
    TEString Name;
    MaterialPassNodeType Type = MaterialPassNodeType::BaseSurfaceSlab;
    bool Enabled = true;
    int TargetQueueIndex = 0;
    TEString QueueName = "Queue 1";

    TEString TexturePath;
    TERef<class Texture> TextureRef;
    TEVector4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
    float FloatVal1 = 1.0f; // e.g. Roughness / Bump Depth / Intensity / Speed X
    float FloatVal2 = 0.0f; // e.g. Metallic / Speed Y
    float FloatVal3 = 1.0f; // e.g. Tiling X
    float FloatVal4 = 1.0f; // e.g. Tiling Y
    int BlendMode = 1;      // 0 = Opaque, 1 = AlphaBlend, 2 = Additive, 3 = Multiply
};

class TE_API Material : public Asset
{
public:
    Material(const TERef<Shader> &shader);
    Material();
    virtual ~Material();

    void SetColor(const TEColor &color);
    const TEColor &GetColor() const;

    void SetUniform(const TEString &name, float value);
    void SetUniform(const TEString &name, int value);
    void SetUniform(const TEString &name, const glm::vec2 &value);
    void SetUniform(const TEString &name, const glm::vec3 &value);
    void SetUniform(const TEString &name, const glm::vec4 &value);
    void SetUniform(const TEString &name, const glm::mat4 &value);

    void SetShader(const TERef<Shader> &shader);
    TERef<Shader> GetShader() const;

    // Set all uniforms (for now, just color and custom ones)
    void ApplyUniforms();

    // Pass Stack Methods
    const TEArray<MaterialPassNode> &GetPassStack() const { return m_PassStack; }
    TEArray<MaterialPassNode> &GetPassStack() { return m_PassStack; }
    void AddPassNode(const MaterialPassNode &node) { m_PassStack.Add(node); }
    void RemovePassNode(size_t index)
    {
        if (index < m_PassStack.Size())
            m_PassStack.RemoveAt(index);
    }

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString type = "Material";
        return type;
    }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override
    {
        static TEString desc = "Material Asset";
        return desc;
    }
    virtual TEString GetDefaultExtension() const override { return ".tematerial"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/MaterialIcon.png"; }

    virtual TERef<class Texture> GetIcon() const override { return nullptr; }
    virtual TERef<class Texture> GetThumbnail() const override { return nullptr; }

    virtual TERef<Asset> Clone() const override { return CreateRef<Material>(nullptr); }
    virtual bool LoadFromFile(const TEString &path) override;

    virtual void OnContentBrowserCreate(const TEString &path) override;

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    static TERef<Material> GetDefault();

private:
    TERef<Shader> m_Shader;
    TEColor m_Color;

    TEMap<TEString, float> m_FloatUniforms;
    TEMap<TEString, int> m_IntUniforms;
    TEMap<TEString, glm::vec2> m_Vec2Uniforms;
    TEMap<TEString, glm::vec3> m_Vec3Uniforms;
    TEMap<TEString, glm::vec4> m_Vec4Uniforms;
    TEMap<TEString, glm::mat4> m_Mat4Uniforms;

    TEArray<MaterialPassNode> m_PassStack;

    AssetHandle m_Handle = 0;
    TEString m_Name = "Unnamed Material";
};

