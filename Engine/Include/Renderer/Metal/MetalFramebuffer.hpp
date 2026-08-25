#pragma once

#include "Renderer/Framebuffer.hpp"


class MetalFramebuffer : public Framebuffer
{
public:
    MetalFramebuffer(const FramebufferSpecification &spec);
    virtual ~MetalFramebuffer();

    void Invalidate();

    virtual void Bind() override;
    virtual void Unbind() override;
    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual uint32_t GetColorAttachmentRendererID() const override { return 0; }
    virtual const FramebufferSpecification &GetSpecification() const override { return m_Specification; }

    void *GetColorTexture() const { return m_ColorTexture; }

private:
    void *m_ColorTexture = nullptr;
    void *m_DepthTexture = nullptr;
    FramebufferSpecification m_Specification;
};

