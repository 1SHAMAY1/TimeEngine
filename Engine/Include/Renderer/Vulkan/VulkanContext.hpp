#pragma once
#include <vector>
#include <volk.h>

namespace TE
{

class VulkanContext
{
public:
    static void Init(void *windowHandle);
    static void Shutdown();

    static VkInstance GetInstance() { return s_Instance; }
    static VkDevice GetDevice() { return s_Device; }
    static VkPhysicalDevice GetPhysicalDevice() { return s_PhysicalDevice; }
    static VkCommandPool GetCommandPool() { return s_CommandPool; }
    static VkQueue GetGraphicsQueue() { return s_GraphicsQueue; }

private:
    static void CreateInstance();
    static void SelectPhysicalDevice();
    static void CreateLogicalDevice();
    static void CreateCommandPool();

private:
    inline static VkInstance s_Instance = VK_NULL_HANDLE;
    inline static VkDevice s_Device = VK_NULL_HANDLE;
    inline static VkPhysicalDevice s_PhysicalDevice = VK_NULL_HANDLE;
    inline static VkQueue s_GraphicsQueue = VK_NULL_HANDLE;
    inline static VkCommandPool s_CommandPool = VK_NULL_HANDLE;
    inline static void *s_WindowHandle = nullptr;
};

} // namespace TE
