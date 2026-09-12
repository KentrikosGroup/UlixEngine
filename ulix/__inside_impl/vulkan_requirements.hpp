#pragma once


#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace __uii::vkreqs {
    constexpr const char* required_extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    constexpr const char* required_validation_layer[] = { "VK_LAYER_KHRONOS_validation" };
    constexpr const char* enabled_extension[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    constexpr VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
}
