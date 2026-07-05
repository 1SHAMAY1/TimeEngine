#pragma once
#include "Renderer/Framebuffer.hpp"
#include <volk.h>

namespace TE {

    class VulkanFramebuffer : public Framebuffer
    {
    public:
        VulkanFramebuffer(const FramebufferSpecification& spec);
        virtual ~VulkanFramebuffer() override;

        void Invalidate();

        virtual void Bind() override;
        virtual void Unbind() override;
        virtual void Resize(uint32_t width, uint32_t height) override;

        virtual uint32_t GetColorAttachmentRendererID() const override { return 0; } // Vulkan image handles are distinct
        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

        VkFramebuffer GetVkFramebuffer() const { return m_Framebuffer; }

    private:
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
        VkImage m_ColorImage = VK_NULL_HANDLE;
        VkImage m_DepthImage = VK_NULL_HANDLE;
        FramebufferSpecification m_Specification;
    };

}
