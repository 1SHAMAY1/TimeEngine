#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/PreRequisites.h"
#include <glm/glm.hpp>

namespace TE
{
class Shader : public Asset
{
public:
    static Shader *Create(const std::string &vertexSrc, const std::string &fragmentSrc);

    virtual ~Shader() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetUniformMat4(const std::string &name, const glm::mat4 &value) = 0;
    virtual void SetUniform4f(const std::string &name, const glm::vec4 &value) = 0;
    virtual void SetUniform3f(const std::string &name, const glm::vec3 &value) = 0;
    virtual void SetUniform2f(const std::string &name, const glm::vec2 &value) = 0;
    virtual void SetUniform1f(const std::string &name, float value) = 0;
    virtual void SetUniform1i(const std::string &name, int value) = 0;

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string& GetType() const override { static std::string type = "Shader"; return type; }
    virtual const std::string& GetName() const override { return m_Name; }
    virtual const std::string& GetHoverDescription() const override { return "Shader Asset"; }
    
    virtual std::shared_ptr<class Texture> GetIcon() const override { return nullptr; }
    virtual std::shared_ptr<class Texture> GetThumbnail() const override { return nullptr; }

protected:
    AssetHandle m_Handle = 0;
    std::string m_Name = "Unnamed Shader";
};
} // namespace TE
