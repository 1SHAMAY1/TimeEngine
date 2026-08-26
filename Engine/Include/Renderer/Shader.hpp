#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/PreRequisites.h"
#include <glm/glm.hpp>

class TE_API Shader : public Asset
{
public:
    static TERef<Shader> Create(const TEString &vertexSrc, const TEString &fragmentSrc);

    virtual ~Shader() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetUniformMat4(const TEString &name, const glm::mat4 &value) = 0;
    virtual void SetUniform4f(const TEString &name, const glm::vec4 &value) = 0;
    virtual void SetUniform3f(const TEString &name, const glm::vec3 &value) = 0;
    virtual void SetUniform2f(const TEString &name, const glm::vec2 &value) = 0;
    virtual void SetUniform1f(const TEString &name, float value) = 0;
    virtual void SetUniform1i(const TEString &name, int value) = 0;

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString type = "Shader";
        return type;
    }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override
    {
        static TEString desc = "Shader Asset";
        return desc;
    }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/ShaderIcon.png"; }

    virtual TERef<class Texture> GetIcon() const override { return nullptr; }
    virtual TERef<class Texture> GetThumbnail() const override { return nullptr; }

    virtual void OnContentBrowserCreate(const TEString &path) override {}

protected:
    AssetHandle m_Handle = 0;
    TEString m_Name = "Unnamed Shader";
};
