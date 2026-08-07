# Vulkan Graphics Backend Architecture

The Vulkan graphics backend in TimeEngine provides explicit low-level Vulkan 1.3 hardware abstraction (`VulkanRendererAPI`), `volk` dynamic loader initialization, device context management ([`VulkanContext`](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/VulkanContext.cpp)), command buffer recording, SPIR-V shader compilation ([`VulkanShader`](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/VulkanShader.cpp)), VkFramebuffer attachments ([`VulkanFramebuffer`](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/VulkanFramebuffer.cpp)), and memory-mapped vertex/index buffers.

> [!NOTE]
> In short, think of the **Vulkan Backend** as TimeEngine's explicit low-overhead next-gen driver layer: `VulkanContext` initializes instance layers, physical GPU selection, logic devices, and command queues; `VulkanRendererAPI` records draw commands into `VkCommandBuffer` instances; while shaders are compiled into SPIR-V bytecodes.

---

## Architecture & Component Breakdown

1. **[`TE::VulkanRendererAPI`](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/VulkanRendererAPI.cpp)**:
   - Derives from `RendererAPI`.
   - Records render pass clears, viewport state pipeline bindings, and `vkCmdDrawIndexed` calls into Vulkan command buffers.
2. **[`TE::VulkanContext`](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/VulkanContext.cpp)**:
   - Manages `VkInstance`, `VkPhysicalDevice`, `VkDevice`, `VkQueue`, and `VkCommandPool`.
3. **[`TE::VulkanShader`](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/VulkanShader.cpp)**:
   - Shader module wrapper initializing `VkShaderModule` from SPIR-V bytecode.
4. **Resource Objects**: `VulkanVertexArray`, `VulkanVertexBuffer`, `VulkanIndexBuffer`, `VulkanFramebuffer`.

---

## Related Architectural Documentation

- [Renderer Subsystem Architecture](../ARCHITECTURE.md) — Multi-backend 2D batching pipeline.
- [DirectX 11 Graphics Backend Architecture](../DirectX11/ARCHITECTURE.md) — Direct3D 11 hardware backend implementation.

