#include "Renderer/TEColor.hpp"
#include "Core/PreRequisites.h"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/TEOpenGLColor.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#include "Utils/MathUtils.hpp"
#include <glm/glm.hpp>
#include <iomanip>

TEColor::TEColor() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}

TEColor::TEColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

TEColor::TEColor(const TEVector4 &color) : r(color.x), g(color.y), b(color.z), a(color.w) {}

TEColor &TEColor::operator=(const TEVector4 &color)
{
    r = color.x;
    g = color.y;
    b = color.z;
    a = color.w;
    return *this;
}

const TEColor &TEColor::Red()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return TEOpenGLColor::Red;
#endif
    default:
        static TEColor fallback = {1, 0, 0, 1};
        return fallback;
    }
}

const TEColor &TEColor::Green()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return TEOpenGLColor::Green;
#endif
    default:
        static TEColor fallback = {0, 1, 0, 1};
        return fallback;
    }
}

const TEColor &TEColor::Blue()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return TEOpenGLColor::Blue;
#endif
    default:
        static TEColor fallback = {0, 0, 1, 1};
        return fallback;
    }
}

const TEColor &TEColor::White()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return TEOpenGLColor::White;
#endif
    default:
        static TEColor fallback = {1, 1, 1, 1};
        return fallback;
    }
}

const TEColor &TEColor::Black()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return TEOpenGLColor::Black;
#endif
    default:
        static TEColor fallback = {0, 0, 0, 1};
        return fallback;
    }
}

const TEColor &TEColor::Transparent()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return TEOpenGLColor::Transparent;
#endif
    default:
        static TEColor fallback = {0, 0, 0, 0};
        return fallback;
    }
}

TEColor TEColor::ToLinear(const TEColor &srgb)
{
    glm::vec3 linear = glm::pow(glm::vec3(srgb.r, srgb.g, srgb.b), glm::vec3(2.2f));
    return TEColor(linear.x, linear.y, linear.z, srgb.a);
}

TEColor TEColor::FromHex(const TEString &hex)
{
    unsigned int r = 0, g = 0, b = 0, a = 255;

    if (hex.Length() == 7 || hex.Length() == 9)
    {
        try
        {
            r = std::stoul(hex.Mid(1, 2).c_str(), nullptr, 16);
            g = std::stoul(hex.Mid(3, 2).c_str(), nullptr, 16);
            b = std::stoul(hex.Mid(5, 2).c_str(), nullptr, 16);
            if (hex.Length() == 9)
            {
                a = std::stoul(hex.Mid(7, 2).c_str(), nullptr, 16);
            }
        }
        catch (...)
        {
        }
    }

    return TEColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}
