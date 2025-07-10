#pragma once
#include "Renderer/Shader.hpp"
#include "Renderer/TEColor.hpp"
#include <memory>
#include <string>

namespace TE {

class TE_API Material {
public:
    Material(const std::shared_ptr<Shader>& shader);
    ~Material();

    void SetColor(const TEColor& color);
    const TEColor& GetColor() const;

    void SetShader(const std::shared_ptr<Shader>& shader);
    std::shared_ptr<Shader> GetShader() const;

    // Set all uniforms (for now, just color)
    void ApplyUniforms();

private:
    std::shared_ptr<Shader> m_Shader;
    TEColor m_Color;
};

} 