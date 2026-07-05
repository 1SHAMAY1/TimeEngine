#include "Renderer/OpenGL/TEOpenGLColor.hpp"

#include "Utils/MathUtils.hpp"

namespace TE {

    const TEColor TEOpenGLColor::Red        = { 1.0f, 0.0f, 0.0f, 1.0f };
    const TEColor TEOpenGLColor::Green      = { 0.0f, 1.0f, 0.0f, 1.0f };
    const TEColor TEOpenGLColor::Blue       = { 0.0f, 0.0f, 1.0f, 1.0f };
    const TEColor TEOpenGLColor::Black      = { 0.0f, 0.0f, 0.0f, 1.0f };
    const TEColor TEOpenGLColor::White      = { 1.0f, 1.0f, 1.0f, 1.0f };
    const TEColor TEOpenGLColor::Transparent= { 0.0f, 0.0f, 0.0f, 0.0f };

}
