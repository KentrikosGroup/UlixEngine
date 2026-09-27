#pragma once

#include "requires.hpp"
#include "vulkan/vulkan_core.h"
#include <format>
#include <print>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef ULIXRELEASE
namespace ulx::log {
    inline static constexpr ulx::strvw vkresult_to_string(VkResult result) {
        switch (result) {
            case VK_ERROR_NOT_PERMITTED: return "VK_ERROR_NOT_PERMITTED";
            case VK_SUCCESS: return "VK_SUCCESS";
            case VK_NOT_READY: return "VK_NOT_READY";
            case VK_TIMEOUT: return "VK_TIMEOUT";
            case VK_EVENT_SET: return "VK_EVENT_SET";
            case VK_EVENT_RESET: return "VK_EVENT_RESET";
            case VK_INCOMPLETE: return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
            case VK_ERROR_VALIDATION_FAILED: return "VK_ERROR_VALIDATION_FAILED|VK_ERROR_VALIDATION_FAILED_EXT";
            case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY|VK_ERROR_OUT_OF_POOL_MEMORY_KHR";
            case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE|VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR";
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS|VK_ERROR_INVALID_DEVICE_ADDRESS_EXT|VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR";
            case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION|VK_ERROR_FRAGMENTATION_EXT";
            case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED|VK_PIPELINE_COMPILE_REQUIRED_EXT|VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT";
            case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
            case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
            case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT: return "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
            case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
            case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
            case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
            case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
            case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
            case VK_INCOMPATIBLE_SHADER_BINARY_EXT: return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
            case VK_PIPELINE_BINARY_MISSING_KHR: return "VK_PIPELINE_BINARY_MISSING_KHR";
            case VK_ERROR_NOT_ENOUGH_SPACE_KHR: return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
            case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
        }
    }
    
    template<typename... Args> [[noreturn]] inline constexpr void err(const char* format, Args&&... args) {
        std::println("ulix: \x1b[91merror:\x1b[0m {}", std::vformat(format, std::make_format_args(args...)));
        std::_Exit(-1);
    }

    template<typename... Args> inline constexpr void expect(bool condition, const char* format, Args&&... args) {
        if (!condition) err(format, std::forward<Args>(args)...);
    }

    template<typename... Args> inline constexpr void vkexp(VkResult result, const char* format, Args&&... args) {
        if (result != VK_SUCCESS) {
            ulx::str error_code = std::format("VkResult: {} ({})", static_cast<ulx::u32>(result), vkresult_to_string(result));
            std::println("ulix: \x1b[91merror:\x1b[0m {}, {}", std::vformat(format, std::make_format_args(args...)), error_code);
            std::_Exit(-1);
        }
    }

    template<typename... Args>
    inline constexpr void warn(const char* format, Args&&... args) {
        std::println("ulix: \x1b[93mwarning:\x1b[0m {}", std::vformat(format, std::make_format_args(args...)));
    }
}
#else
namespace ulx::log {
    template<typename... Args> [[noreturn]] inline constexpr void err([[maybe_unused]] const char* format, [[maybe_unused]] Args&&... args) { std::_Exit(-1); }
    template<typename... Args> inline constexpr void expect([[maybe_unused]] bool condition, [[maybe_unused]] const char* format, [[maybe_unused]] Args&&... args) {}
    template<typename... Args> inline constexpr void warn([[maybe_unused]] const char* format, [[maybe_unused]] Args&&... args) {}
    template<typename... Args> inline constexpr void vkexp([[maybe_unused]] VkResult result, [[maybe_unused]] const char* format, [[maybe_unused]] Args&&... args) {}
}
#endif
