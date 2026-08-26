#include "Core/PreRequisites.h"
#include "Renderer/Material.hpp"
#include "Core/Log.h"
#include "Renderer/MaterialSerializer.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include "Utils/TEFileSystem.hpp"

#include "Utils/TEFileSystem.hpp"
TE_REGISTER_ASSET(Material)

bool Material::LoadFromFile(const TEString &path)
{
    auto self = TERef<Material>(this, [](Material *) {});
    MaterialSerializer serializer(self);
    return serializer.Deserialize(path);
}

Material::Material() : m_Shader(nullptr), m_Color(TEColor::White())
{
    // Add default Base Surface Slab
    MaterialPassNode baseNode;
    baseNode.Name = "Base Surface Slab";
    baseNode.Type = MaterialPassNodeType::BaseSurfaceSlab;
    baseNode.Enabled = true;
    baseNode.Color = {1.0f, 1.0f, 1.0f, 1.0f};
    baseNode.FloatVal1 = 0.5f; // Roughness
    baseNode.FloatVal2 = 0.0f; // Metallic
    m_PassStack.Add(baseNode);

    // Add default Blend Output State
    MaterialPassNode blendNode;
    blendNode.Name = "Blend Output State";
    blendNode.Type = MaterialPassNodeType::BlendOutputState;
    blendNode.Enabled = true;
    blendNode.BlendMode = 1; // AlphaBlend
    m_PassStack.Add(blendNode);
}

Material::Material(const TERef<Shader> &shader) : m_Shader(shader), m_Color(TEColor::White())
{
    MaterialPassNode baseNode;
    baseNode.Name = "Base Surface Slab";
    baseNode.Type = MaterialPassNodeType::BaseSurfaceSlab;
    baseNode.Enabled = true;
    baseNode.Color = {1.0f, 1.0f, 1.0f, 1.0f};
    baseNode.FloatVal1 = 0.5f;
    baseNode.FloatVal2 = 0.0f;
    m_PassStack.Add(baseNode);

    MaterialPassNode blendNode;
    blendNode.Name = "Blend Output State";
    blendNode.Type = MaterialPassNodeType::BlendOutputState;
    blendNode.Enabled = true;
    blendNode.BlendMode = 1;
    m_PassStack.Add(blendNode);
}

Material::~Material() {}

TERef<Material> Material::GetDefault()
{
    static TERef<Material> s_DefaultMaterial = nullptr;
    if (!s_DefaultMaterial)
    {
        s_DefaultMaterial = CreateRef<Material>(ShaderLibrary::CreateColorShader());
        s_DefaultMaterial->SetName("Default Material");
    }
    return s_DefaultMaterial;
}

void Material::SetColor(const TEColor &color) { m_Color = color; }

const TEColor &Material::GetColor() const { return m_Color; }

void Material::SetShader(const TERef<Shader> &shader) { m_Shader = shader; }

TERef<Shader> Material::GetShader() const { return m_Shader; }

void Material::SetUniform(const TEString &name, float value) { m_FloatUniforms[name] = value; }
void Material::SetUniform(const TEString &name, int value) { m_IntUniforms[name] = value; }
void Material::SetUniform(const TEString &name, const glm::vec2 &value) { m_Vec2Uniforms[name] = value; }
void Material::SetUniform(const TEString &name, const glm::vec3 &value) { m_Vec3Uniforms[name] = value; }
void Material::SetUniform(const TEString &name, const glm::vec4 &value) { m_Vec4Uniforms[name] = value; }
void Material::SetUniform(const TEString &name, const glm::mat4 &value) { m_Mat4Uniforms[name] = value; }

void Material::ApplyUniforms()
{
    if (m_Shader)
    {
        ShaderLibrary::SetColor(m_Shader.get(), m_Color);

        for (auto const &[name, val] : m_FloatUniforms)
            m_Shader->SetUniform1f(name, val);
        for (auto const &[name, val] : m_IntUniforms)
            m_Shader->SetUniform1i(name, val);
        for (auto const &[name, val] : m_Vec2Uniforms)
            m_Shader->SetUniform2f(name, val);
        for (auto const &[name, val] : m_Vec3Uniforms)
            m_Shader->SetUniform3f(name, val);
        for (auto const &[name, val] : m_Vec4Uniforms)
            m_Shader->SetUniform4f(name, val);
        for (auto const &[name, val] : m_Mat4Uniforms)
            m_Shader->SetUniformMat4(name, val);
    }
}

void Material::OnContentBrowserCreate(const TEString &path)
{
    TEFileSystem::CreateDirectories(path);
    TEString baseName = "NewMaterial";
    TEString finalPath = path / (baseName + ".tematerial");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tematerial");
    }

    auto newMaterial = CreateRef<Material>(nullptr);
    newMaterial->SetName(finalPath.GetStem());
    MaterialSerializer serializer(newMaterial);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Material at {0}", finalPath.c_str());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create Material at {0}", finalPath.c_str());
    }
}
