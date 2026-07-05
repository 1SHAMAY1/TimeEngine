#include "Renderer/Vulkan/VulkanFramebuffer.hpp"
#include "Renderer/Vulkan/VulkanContext.hpp"
#include <iostream>

namespace TE {

    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec)
    {
        Invalidate();
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        VkDevice device = VulkanContext::GetDevice();
        if (device != VK_NULL_HANDLE) {
            if (m_Framebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
            }
            if (m_ColorImage != VK_NULL_HANDLE) {
                vkDestroyImage(device, m_ColorImage, nullptr);
            }
            if (m_DepthImage != VK_NULL_HANDLE) {
                vkDestroyImage(device, m_DepthImage, nullptr);
            }
        }
    }

    void VulkanFramebuffer::Invalidate()
    {
        VkDevice device = VulkanContext::GetDevice();
        if (device == VK_NULL_HANDLE) {
            return;
        }

        // Clean up existing resources if any
        if (m_Framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
            m_Framebuffer = VK_NULL_HANDLE;
        }
        if (m_ColorImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_ColorImage, nullptr);
            m_ColorImage = VK_NULL_HANDLE;
        }
        if (m_DepthImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_DepthImage, nullptr);
            m_DepthImage = VK_NULL_HANDLE;
        }

        // Create color attachment image
        VkImageCreateInfo colorImageInfo{};
        colorImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        colorImageInfo.imageType = VK_IMAGE_TYPE_2D;
        colorImageInfo.extent.width = m_Specification.Width;
        colorImageInfo.extent.height = m_Specification.Height;
        colorImageInfo.extent.depth = 1;
        colorImageInfo.mipLevels = 1;
        colorImageInfo.arrayLayers = 1;
        colorImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        colorImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        colorImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        colorImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        colorImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &colorImageInfo, nullptr, &m_ColorImage) != VK_SUCCESS) {
            std::cout << "[Vulkan] Error: Failed to create framebuffer color attachment\n";
        }
    }

    void VulkanFramebuffer::Bind()
    {
        // Handled via RenderPass command buffer recording
    }

    void VulkanFramebuffer::Unbind()
    {
    }

    void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }

}
