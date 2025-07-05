#include "Renderer/RenderCommand.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/GraphicsAPI.hpp"

#include <glad/glad.h>

namespace TE {

    void RenderCommand::Init() {
        // Any API-specific initialization
        if (RendererContext::GetAPI() == GraphicsAPI::OpenGL) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    void RenderCommand::SetClearColor(const glm::vec4& color) {
        if (RendererContext::GetAPI() == GraphicsAPI::OpenGL) {
            glClearColor(color.r, color.g, color.b, color.a);
        }
    }

    void RenderCommand::Clear() {
        if (RendererContext::GetAPI() == GraphicsAPI::OpenGL) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }

    void RenderCommand::DrawIndexed(unsigned int vao, unsigned int indexCount) {
        if (RendererContext::GetAPI() == GraphicsAPI::OpenGL) {
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        }
    }
}
