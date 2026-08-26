#include "Core/PreRequisites.h"
#include "Renderer/Vulkan/TEVulkanColor.hpp"
#include "Utils/MathUtils.hpp"

const TEColor TEVulkanColor::Red = {1.0f, 0.0f, 0.0f, 1.0f};
const TEColor TEVulkanColor::Green = {0.0f, 1.0f, 0.0f, 1.0f};
const TEColor TEVulkanColor::Blue = {0.0f, 0.0f, 1.0f, 1.0f};
const TEColor TEVulkanColor::Black = {0.0f, 0.0f, 0.0f, 1.0f};
const TEColor TEVulkanColor::White = {1.0f, 1.0f, 1.0f, 1.0f};
const TEColor TEVulkanColor::Transparent = {0.0f, 0.0f, 0.0f, 0.0f};
