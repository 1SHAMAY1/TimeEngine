#include "Renderer/OpenGLES/OpenGLESFramebuffer.hpp"
#include "Core/Log.h"
#include <glad/glad.h>

namespace TE
{

static const uint32_t s_MaxFramebufferSize = 8192;

// ---------------------------------------------------------------------------
// OpenGLESFramebuffer
// ---------------------------------------------------------------------------

OpenGLESFramebuffer::OpenGLESFramebuffer(const FramebufferSpecification &spec) : m_Specification(spec) { Invalidate(); }

OpenGLESFramebuffer::~OpenGLESFramebuffer()
{
    glDeleteFramebuffers(1, &m_RendererID);
    glDeleteTextures(1, &m_ColorAttachment);
    glDeleteRenderbuffers(1, &m_DepthAttachment);
}

void OpenGLESFramebuffer::Invalidate()
{
    if (m_RendererID)
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(1, &m_ColorAttachment);
        glDeleteRenderbuffers(1, &m_DepthAttachment);
        m_RendererID = 0;
        m_ColorAttachment = 0;
        m_DepthAttachment = 0;
    }

    // --- Color attachment: RGBA8 texture ---
    glGenFramebuffers(1, &m_RendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

    glGenTextures(1, &m_ColorAttachment);
    glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(m_Specification.Width),
                 static_cast<GLsizei>(m_Specification.Height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

    // --- Depth+Stencil attachment: Renderbuffer (ES prefers renderbuffers for depth) ---
    glGenRenderbuffers(1, &m_DepthAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(m_Specification.Width),
                          static_cast<GLsizei>(m_Specification.Height));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        TE_CORE_ERROR("OpenGL ES Framebuffer is incomplete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLESFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
    glViewport(0, 0, static_cast<GLsizei>(m_Specification.Width), static_cast<GLsizei>(m_Specification.Height));
}

void OpenGLESFramebuffer::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void OpenGLESFramebuffer::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
    {
        TE_CORE_WARN("Attempted to resize OpenGL ES framebuffer to {0}, {1}", width, height);
        return;
    }

    m_Specification.Width = width;
    m_Specification.Height = height;
    Invalidate();
}

} // namespace TE
