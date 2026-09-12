#pragma once


#include "../pixmap.hpp"
#include "../__inside_impl/vulkan_classes.hpp"
#include "../color.hpp"
#include "../log.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <fstream>

namespace __uii::vkalg {
    inline static constexpr glm::vec4 color_to_vec4(const ulx::color& color) {
        return glm::vec4(color.get_red() / 255.0f,
                         color.get_green() / 255.0f,
                         color.get_blue() / 255.0f,
                         color.get_alpha() / 255.0f);
    }

    inline auto get_device_score(const __uii::vkclses::PhysicalDeviceInfos& device_infos) -> ulx::u32 {
        ulx::u64 device_local_bytes = 0;
        for (ulx::u32 index = 0; index < device_infos.memory_properties.memoryHeapCount; ++index) {
            if (!(device_infos.memory_properties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)) continue;
            device_local_bytes += device_infos.memory_properties.memoryHeaps[index].size;
        }

        return device_infos.properties.limits.maxImageDimension2D +
            device_infos.properties.limits.maxUniformBufferRange / (1 << 20) +
            device_infos.properties.limits.maxComputeWorkGroupCount[0] / 65536 +
            device_infos.properties.limits.maxComputeWorkGroupInvocations / 1024 +
            device_infos.properties.limits.maxBoundDescriptorSets +
            device_infos.properties.limits.maxColorAttachments +
            device_local_bytes / (1ULL << 30);
    }

    inline auto find_memory_type(VkPhysicalDevice device, ulx::u32 type_filter, VkMemoryPropertyFlags properties) -> ulx::u32 {
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(device, &physical_device_memory_properties);

        for (ulx::u32 index = 0; index < physical_device_memory_properties.memoryTypeCount; index++) {
            if ((type_filter & (1 << index)) && (physical_device_memory_properties.memoryTypes[index].propertyFlags & properties) == properties) {
                return index;
            }
        }

        ulx::log::ulixerr("Failed to find suitable memory type");
    }

    inline auto start_single_time_command_buffer(VkDevice device, VkCommandPool command_pool) -> VkCommandBuffer {
        VkCommandBuffer command_buffer;

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;
        command_buffer_allocate_info.commandPool = command_pool;
        if (vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer))
            ulx::log::ulixerr("Failed to allocate single time command buffer");

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info))
            ulx::log::ulixerr("Failed to begin single time command buffer");

        return command_buffer;
    }

    inline auto end_single_time_command_buffer(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkCommandBuffer command_buffer) -> void {
        if (vkEndCommandBuffer(command_buffer))
            ulx::log::ulixerr("Failed to end single time command buffer");

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        if (vkQueueSubmit(queue, 1, &submit_info, nullptr))
            ulx::log::ulixerr("Failed to submit single time command buffer");

        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    }

    inline auto transition_image_layout(VkDevice device, VkCommandBuffer single_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) -> void {
        VkPipelineStageFlags src_stage;
        VkPipelineStageFlags dst_stage;

        VkImageMemoryBarrier image_memory_barrier{};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.oldLayout = old_layout;
        image_memory_barrier.newLayout = new_layout;
        image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.image = image;
        image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_memory_barrier.subresourceRange.baseMipLevel = 0;
        image_memory_barrier.subresourceRange.levelCount = 1;
        image_memory_barrier.subresourceRange.baseArrayLayer = 0;
        image_memory_barrier.subresourceRange.layerCount = 1;
        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else ulx::log::ulixerr("Invalid layout format");

        vkCmdPipelineBarrier(single_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    }

    inline auto copy_buffer_to_image(VkDevice device, VkBuffer dst_buffer, VkCommandBuffer single_buffer, VkImage image, ulx::u32 width, ulx::u32 height, VkDeviceSize buffer_offset) -> void {
        VkBufferImageCopy buffer_image_copy_region{};
        buffer_image_copy_region.bufferOffset = buffer_offset;
        buffer_image_copy_region.bufferRowLength = 0;
        buffer_image_copy_region.bufferImageHeight = 0;
        buffer_image_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_image_copy_region.imageSubresource.mipLevel = 0;
        buffer_image_copy_region.imageSubresource.baseArrayLayer = 0;
        buffer_image_copy_region.imageSubresource.layerCount = 1;
        buffer_image_copy_region.imageOffset = {0, 0, 0};
        buffer_image_copy_region.imageExtent = {width, height, 1};
        vkCmdCopyBufferToImage(single_buffer, dst_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy_region);
    }

    inline auto create_image_view(VkDevice device, VkImage image, VkFormat format) -> VkImageView {
        VkImageView image_view;
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        if (vkCreateImageView(device, &image_view_create_info, nullptr, &image_view) != VK_SUCCESS)
            ulx::log::ulixerr("Failed to create image view");

        return image_view;
    }

    inline auto empty_pixmap() -> ulx::pixmap {
        return ulx::pixmap(ulx::vec<ulx::u8>{0, 0, 0, 0}, 1, 1);
    }

    /* ============================== File Algorithm ============================== */
    inline auto read_cache(const ulx::str& file_path) -> ulx::vec<char> {
        std::ifstream file = std::ifstream(file_path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) ulx::log::ulixerr("Failed to read cache");
        size_t file_size = (size_t) file.tellg();
        ulx::vec<char> buffer = ulx::vec<char>(file_size);
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), file_size);
        file.close();
        return buffer;
    }

    inline void write_cache(const ulx::str& file_path, const ulx::vec<char>& buffer) {
        std::ofstream file = std::ofstream(file_path, std::ios::binary);
        if (!file.is_open()) ulx::log::ulixerr("Failed to write cache");
        file.write(buffer.data(), buffer.size());
        file.close();
    }

    inline void ensure_cache_exists(const ulx::str& file_path) {
        if (!std::filesystem::exists(file_path))
            write_cache(file_path, {});
    }
}
