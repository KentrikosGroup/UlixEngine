#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <windows.h>
#include <vulkan/vulkan.h>
#include <ulix/types.hpp>
#include <ulix/__inside_impl/log_utils.hpp>

namespace ulx::vkc {
    struct queue_family_indices {
        opt<u32> graphics_queue_family_index;
        opt<u32> present_queue_family_index;

        inline auto is_complete() const -> bool {
            return graphics_queue_family_index.has_value() &&
                    present_queue_family_index.has_value();
        }
    };

    struct swapchain_support_details {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vec<VkSurfaceFormatKHR> surface_formats;
        vec<VkPresentModeKHR> present_modes;

        swapchain_support_details() = default;
        swapchain_support_details(VkPhysicalDevice device, VkSurfaceKHR window_surface) {
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window_surface, &surface_capabilities);

            u32 surface_format_count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, window_surface, &surface_format_count, VK_NULL_HANDLE);
            if (surface_format_count) {
                surface_formats.resize(surface_format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, window_surface, &surface_format_count, surface_formats.data());
            }

            u32 present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, window_surface, &present_mode_count, VK_NULL_HANDLE);
            if (present_mode_count) {
                present_modes.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, window_surface, &present_mode_count, present_modes.data());
            }
        }

        inline auto is_adequate() -> bool {
            return !surface_formats.empty() &&
                    !present_modes.empty();
        }
    };

    struct physical_device_info {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memory_properties;
        VkPhysicalDeviceFeatures features;
        queue_family_indices queue_family_indices;
        swapchain_support_details swapchain_details;
        bool extension_support;

        physical_device_info() = default;
        physical_device_info(VkPhysicalDevice device, VkSurfaceKHR surface) {
            vkGetPhysicalDeviceProperties(device, &properties);
            vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);
            vkGetPhysicalDeviceFeatures(device, &features);

            u32 queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, VK_NULL_HANDLE);
            vec<VkQueueFamilyProperties> queue_family_properties_list = vec<VkQueueFamilyProperties>(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_properties_list.data());

            for (u64 i = 0; i < queue_family_count; i++) {
                if (queue_family_properties_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    queue_family_indices.graphics_queue_family_index = i;

                VkBool32 present_queue_family_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_queue_family_support);
                if (present_queue_family_support) queue_family_indices.present_queue_family_index = i;
                if (queue_family_indices.is_complete()) break;
            }

            extension_support = check_extension_support(device);
            if (!extension_support) return;
            swapchain_details = swapchain_support_details(device, surface);
        }

        inline auto get_unique_queue_family_indices() -> set<u32> {
            set<u32> unique_queue_family_indices = {
                queue_family_indices.graphics_queue_family_index.value(),
                queue_family_indices.present_queue_family_index.value()
            };

            return unique_queue_family_indices;
        }

        inline auto match_gpu_type(VkPhysicalDeviceType gpu_type) const -> bool {
            return properties.deviceType == gpu_type;
        }

        inline auto is_suitable() -> bool {
            return (
                 match_gpu_type(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) ||
                 match_gpu_type(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            ) && features.geometryShader && features.tessellationShader &&
                 queue_family_indices.is_complete() &&
                 extension_support &&
                 swapchain_details.is_adequate() &&
                 features.samplerAnisotropy;
        }

        inline auto check_extension_support(VkPhysicalDevice device) -> bool {
            u32 extension_count = 0;
            vkEnumerateDeviceExtensionProperties(device, VK_NULL_HANDLE, &extension_count, VK_NULL_HANDLE);
            vec<VkExtensionProperties> available_extensions = vec<VkExtensionProperties>(extension_count);
            vkEnumerateDeviceExtensionProperties(device, VK_NULL_HANDLE, &extension_count, available_extensions.data());

            for (const auto& extension : available_extensions) {
                if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
                    return true;
            }
            return false;
        }
    };

    struct device_queues {
        VkQueue graphics_queue;
        VkQueue present_queue;

        device_queues() = default;
        device_queues(VkDevice logical_device, const physical_device_info& device_infos) {
            vkGetDeviceQueue(logical_device, device_infos.queue_family_indices.graphics_queue_family_index.value(), 0, &graphics_queue);
            vkGetDeviceQueue(logical_device, device_infos.queue_family_indices.present_queue_family_index.value(), 0, &present_queue);
        }
    };

    struct buffer {
        VkBuffer       m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory;
        VkDeviceSize   m_size;

        buffer() = default;
        buffer(VkPhysicalDevice physical_device, VkDevice logical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties): m_size(size) {
            VkBufferCreateInfo buffer_create_info{};
            buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_create_info.size = size;
            buffer_create_info.usage = usage;
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            log::uassert(vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &m_buffer) == VK_SUCCESS,
                "create buffer failed");

            VkMemoryRequirements buffer_memory_requirements;
            vkGetBufferMemoryRequirements(logical_device, m_buffer, &buffer_memory_requirements);

            VkMemoryAllocateInfo buffer_memory_allocate_info{};
            buffer_memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            buffer_memory_allocate_info.allocationSize = buffer_memory_requirements.size;
            buffer_memory_allocate_info.memoryTypeIndex = find_memory_type(physical_device, buffer_memory_requirements.memoryTypeBits, properties);
            log::uassert(vkAllocateMemory(logical_device, &buffer_memory_allocate_info, nullptr, &m_memory) == VK_SUCCESS,
                "allocate buffer memory failed");

            log::uassert(vkBindBufferMemory(logical_device, m_buffer, m_memory, 0) == VK_SUCCESS, 
                "bind buffer memory failed");
        }

        inline constexpr auto map_memory(VkDevice device, const void* data, VkDeviceSize size) -> void {
            void* dst_data;
            vkMapMemory(device, m_memory, 0, size, 0, &dst_data);
                memcpy(dst_data, data, static_cast<u64>(size));
            vkUnmapMemory(device, m_memory);
        }

        inline constexpr auto copy_buffer_to(const buffer& buffer, VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue, VkDeviceSize size) -> void {
            VkCommandBufferAllocateInfo command_buffer_allocate_info{};
            command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            command_buffer_allocate_info.commandBufferCount = 1;
            command_buffer_allocate_info.commandPool = command_pool;

            VkCommandBuffer command_buffer;
            vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer);

            VkCommandBufferBeginInfo command_buffer_begin_info{};
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

            VkBufferCopy buffer_copy_region{};
            buffer_copy_region.srcOffset = 0;
            buffer_copy_region.dstOffset = 0;
            buffer_copy_region.size = size;
            vkCmdCopyBuffer(command_buffer, m_buffer, buffer.m_buffer, 1, &buffer_copy_region);
            vkEndCommandBuffer(command_buffer);

            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;
            vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
            vkQueueWaitIdle(graphics_queue);

            vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        }

        inline constexpr auto release(VkDevice device) -> void {
            vkDestroyBuffer(device, m_buffer, nullptr);
            vkFreeMemory(device, m_memory, nullptr);
        }

        inline static auto find_memory_type(VkPhysicalDevice device, u32 type_filter, VkMemoryPropertyFlags properties) -> u32 {
            VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
            vkGetPhysicalDeviceMemoryProperties(device, &physical_device_memory_properties);

            for (u32 index = 0; index < physical_device_memory_properties.memoryTypeCount; index++) {
                if ((type_filter & (1 << index)) && (physical_device_memory_properties.memoryTypes[index].propertyFlags & properties) == properties) {
                    return index;
                }
            }

            log::uerror("find suitable memory type failed");
        }

        inline constexpr auto valid() const -> bool {
            return m_buffer != VK_NULL_HANDLE;
        }
    };

    struct image {
        VkImage m_image;
        VkImageView m_image_view;

        inline image() = default;
        inline auto release(VkDevice device) -> void {
            vkDestroyImageView(device, m_image_view, VK_NULL_HANDLE);
            vkDestroyImage(device, m_image, VK_NULL_HANDLE);
            m_image = VK_NULL_HANDLE;
            m_image_view = VK_NULL_HANDLE;
        }
    };
}
