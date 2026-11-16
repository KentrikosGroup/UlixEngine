#pragma once
#include <glm/glm.hpp>
#include <ulix/pixmap.hpp>
#include <ulix/color.hpp>
#include <ulix/path.hpp>
#include <ulix/__inside_impl/vulkan_classes.hpp>
#include <ulix/__inside_impl/log_utils.hpp>

namespace ulx::vka {
    inline auto get_device_score(const vkc::physical_device_info& device_infos) -> u32 {
        u64 device_local_bytes = 0;
        for (u32 index = 0; index < device_infos.memory_properties.memoryHeapCount; ++index) {
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

    inline auto find_memory_type(VkPhysicalDevice device, u32 type_filter, VkMemoryPropertyFlags properties) -> u32 {
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(device, &physical_device_memory_properties);

        for (u32 index = 0; index < physical_device_memory_properties.memoryTypeCount; index++) {
            if ((type_filter & (1 << index)) && (physical_device_memory_properties.memoryTypes[index].propertyFlags & properties) == properties) {
                return index;
            }
        }

        log::uerror("find suitable memory type failed");
    }

    inline auto start_single_time_command_buffer(VkDevice device, VkCommandPool command_pool) -> VkCommandBuffer {
        VkCommandBuffer command_buffer;

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;
        command_buffer_allocate_info.commandPool = command_pool;
        log::uassert(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer) == VK_SUCCESS, 
            "allocate single time command buffer failed");

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        log::uassert(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) == VK_SUCCESS, 
            "begin single time command buffer failed");

        return command_buffer;
    }

    inline auto end_single_time_command_buffer(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkCommandBuffer command_buffer) -> void {
        log::uassert(vkEndCommandBuffer(command_buffer) == VK_SUCCESS, 
            "end single time command buffer failed");

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        log::uassert(vkQueueSubmit(queue, 1, &submit_info, nullptr) == VK_SUCCESS, 
            "submit single time command buffer failed");

        log::uassert(vkQueueWaitIdle(queue) == VK_SUCCESS, "wait queue idle failed");
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
        } else log::uerror("invalid layout format: old layout: {}, new layout: {}", (u32)old_layout, (u32)new_layout);
        vkCmdPipelineBarrier(single_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    }

    inline auto copy_buffer_to_image(VkDevice device, VkBuffer dst_buffer, VkCommandBuffer single_buffer, VkImage image, u32 width, u32 height, VkDeviceSize buffer_offset) -> void {
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
        log::uassert(vkCreateImageView(device, &image_view_create_info, nullptr, &image_view) == VK_SUCCESS, 
            "create image view failed");

        return image_view;
    }
}
