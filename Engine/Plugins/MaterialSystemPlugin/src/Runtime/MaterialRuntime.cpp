#include "Runtime/MaterialRuntime.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"


void MaterialRuntime::BindMaterial(const TERef<MaterialAsset> &material, float time,
                                   const TEVector2 &lightPos, const TEVector4 &lightColor,
                                   float lightIntensity)
{
    if (!material)
        return;

    auto shader = material->GetCompiledShader();
    if (!shader)
        return;

    shader->Bind();
    shader->SetUniform1f("u_Time", time);
    shader->SetUniform2f("u_LightPos", {lightPos.x, lightPos.y});
    shader->SetUniform4f("u_LightColor", {lightColor.x, lightColor.y, lightColor.z, lightColor.w});
    shader->SetUniform1f("u_LightIntensity", lightIntensity);

    const auto &uniforms = material->GetCompileResult().Uniforms;
    for (const auto &pair : uniforms.ScalarParameters)
    {
        TEString uName = TEString("u_") + pair.second.Name;
        shader->SetUniform1f(uName, pair.second.DefaultValue);
    }
    for (const auto &pair : uniforms.VectorParameters)
    {
        TEString uName = TEString("u_") + pair.second.Name;
        const auto &v = pair.second.DefaultValue;
        shader->SetUniform4f(uName, {v.x, v.y, v.z, v.w});
    }
}

void MaterialRuntime::BindMaterialInstance(const TERef<MaterialInstanceAsset> &instance,
                                           const TERef<MaterialAsset> &parentMaterial,
                                           float time, const TEVector2 &lightPos,
                                           const TEVector4 &lightColor, float lightIntensity)
{
    if (!instance || !parentMaterial)
        return;

    auto shader = parentMaterial->GetCompiledShader();
    if (!shader)
        return;

    shader->Bind();
    shader->SetUniform1f("u_Time", time);
    shader->SetUniform2f("u_LightPos", {lightPos.x, lightPos.y});
    shader->SetUniform4f("u_LightColor", {lightColor.x, lightColor.y, lightColor.z, lightColor.w});
    shader->SetUniform1f("u_LightIntensity", lightIntensity);

    const auto &uniforms = parentMaterial->GetCompileResult().Uniforms;

    for (const auto &pair : uniforms.ScalarParameters)
    {
        TEString uName = TEString("u_") + pair.second.Name;
        float val = instance->GetScalarOverride(pair.second.Name, pair.second.DefaultValue);
        shader->SetUniform1f(uName, val);
    }
    for (const auto &pair : uniforms.VectorParameters)
    {
        TEString uName = TEString("u_") + pair.second.Name;
        TEVector4 v = instance->GetVectorOverride(pair.second.Name, pair.second.DefaultValue);
        shader->SetUniform4f(uName, {v.x, v.y, v.z, v.w});
    }
}
