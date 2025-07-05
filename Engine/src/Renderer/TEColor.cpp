#include "Renderer/TEColor.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/OpenGL/TEOpenGLColor.hpp"
#include <sstream>
#include <iomanip>

namespace TE {

    TEColor::TEColor()
        : m_Value(0.0f, 0.0f, 0.0f, 1.0f) {}

    TEColor::TEColor(float r, float g, float b, float a)
        : m_Value(r, g, b, a) {}

    TEColor::TEColor(const glm::vec4& color)
        : m_Value(color) {}

    const glm::vec4& TEColor::Red() {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return TEOpenGLColor::Red;
        default: static glm::vec4 fallback = {1, 0, 0, 1}; return fallback;
        }
    }

    const glm::vec4& TEColor::Green()
    {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return TEOpenGLColor::Green;
        default: static glm::vec4 fallback = {1, 0, 0, 1}; return fallback;
        }
    }

    const glm::vec4& TEColor::Blue()
    {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return TEOpenGLColor::Blue;
        default: static glm::vec4 fallback = {1, 0, 0, 1}; return fallback;
        }
    }

    const glm::vec4& TEColor::White()
    {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return TEOpenGLColor::White;
        default: static glm::vec4 fallback = {1, 0, 0, 1}; return fallback;
        }
    }

    const glm::vec4& TEColor::Black()
    {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return TEOpenGLColor::Black;
        default: static glm::vec4 fallback = {1, 0, 0, 1}; return fallback;
        }
    }

    const glm::vec4& TEColor::Transparent()
    {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return TEOpenGLColor::Transparent;
        default: static glm::vec4 fallback = {1, 0, 0, 1}; return fallback;
        }
    }


    glm::vec4 TEColor::ToLinear(const glm::vec4& srgb) {
        glm::vec3 linear = glm::pow(glm::vec3(srgb), glm::vec3(2.2f));
        return glm::vec4(linear, srgb.a);
    }

    TEColor TEColor::FromHex(const std::string& hex) {
        unsigned int r = 0, g = 0, b = 0, a = 255;

        if (hex.length() == 7 || hex.length() == 9) {
            std::stringstream ss;
            ss << std::hex << hex.substr(1, 2); ss >> r; ss.clear();
            ss << std::hex << hex.substr(3, 2); ss >> g; ss.clear();
            ss << std::hex << hex.substr(5, 2); ss >> b;

            if (hex.length() == 9) {
                ss.clear();
                ss << std::hex << hex.substr(7, 2); ss >> a;
            }
        }

        return TEColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

}
