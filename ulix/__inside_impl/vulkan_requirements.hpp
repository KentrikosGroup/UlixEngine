#pragma once
#include "ulix/types.hpp"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace __uii::vkreqs {
    constexpr const char* required_extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };
    constexpr ulx::u32 required_extension_count = 3;
    constexpr const char* required_validation_layer[] = { "VK_LAYER_KHRONOS_validation" };
    constexpr const char* enabled_extensions[] = { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME 
    };
    constexpr ulx::u32 enabled_extension_count = 3;
    constexpr VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
}
