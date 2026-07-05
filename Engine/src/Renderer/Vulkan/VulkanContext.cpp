#include "Renderer/Vulkan/VulkanContext.hpp"
#include <iostream>
#include <vector>

namespace TE
{

void VulkanContext::Init(void *windowHandle)
{
    s_WindowHandle = windowHandle;
    std::cout << "[Vulkan] Initializing Context\n";
    CreateInstance();
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateCommandPool();
}

void VulkanContext::Shutdown()
{
    std::cout << "[Vulkan] Shutting down Context\n";
    if (s_CommandPool)
    {
        vkDestroyCommandPool(s_Device, s_CommandPool, nullptr);
        s_CommandPool = VK_NULL_HANDLE;
    }
    if (s_Device)
    {
        vkDestroyDevice(s_Device, nullptr);
        s_Device = VK_NULL_HANDLE;
    }
    if (s_Instance)
    {
        vkDestroyInstance(s_Instance, nullptr);
        s_Instance = VK_NULL_HANDLE;
    }
}

void VulkanContext::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "TimeEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "TimeEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // On Windows/GLFW, surface extensions are required
    std::vector<const char *> extensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"};
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // In a real scenario, we call vkCreateInstance.
    // We compile/stub the call so that if the user doesn't link vulkan-1.lib, we don't get link errors.
    // We dynamically load them or stub the call for compile safety.
    // Let's implement it as a standard call, assuming the user links or loads Vulkan.
    // If not linking, they can link vulkan-1.lib or resolve via glfwGetInstanceProcAddress.
}

void VulkanContext::SelectPhysicalDevice()
{
    // Find physical GPU
}

void VulkanContext::CreateLogicalDevice()
{
    // Create logical device
}

void VulkanContext::CreateCommandPool()
{
    // Create command pool
}

} // namespace TE
