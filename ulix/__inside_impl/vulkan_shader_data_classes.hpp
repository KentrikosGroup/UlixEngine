#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "../types.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace __uii::vsdces {
    struct vertex {
        glm::vec2 position;
        glm::vec4 color;
        ulx::f32 corner_radius;
        glm::vec2 half_size;
        ulx::f32 border_width;
        glm::vec4 border_color;
        glm::vec2 center;
        glm::vec2 uv;
        ulx::u32 tex_index;

        inline static auto get_binding_description() -> VkVertexInputBindingDescription {
            VkVertexInputBindingDescription binding_description{};
            binding_description.binding = 0;
            binding_description.stride = sizeof(vertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return binding_description;
        }

        template<typename M>
        inline static auto new_attrdesc(ulx::u32 location, VkFormat format, M vertex::*member) -> VkVertexInputAttributeDescription {
            VkVertexInputAttributeDescription vertex_input_attribute_description{};
            vertex_input_attribute_description.location = location;
            vertex_input_attribute_description.binding = 0;
            vertex_input_attribute_description.format = format;
            vertex_input_attribute_description.offset = static_cast<ulx::u32>(reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<vertex const volatile*>(0)->*member)));

            return vertex_input_attribute_description;
        }

        inline static auto get_attribute_descriptions() -> ulx::vec<VkVertexInputAttributeDescription> {
            ulx::vec<VkVertexInputAttributeDescription> attribute_descriptions;
            attribute_descriptions.push_back(new_attrdesc(0, VK_FORMAT_R32G32_SFLOAT, &vertex::position));
            attribute_descriptions.push_back(new_attrdesc(1, VK_FORMAT_R32G32B32A32_SFLOAT, &vertex::color));
            attribute_descriptions.push_back(new_attrdesc(2, VK_FORMAT_R32_SFLOAT, &vertex::corner_radius));
            attribute_descriptions.push_back(new_attrdesc(3, VK_FORMAT_R32G32_SFLOAT, &vertex::half_size));
            attribute_descriptions.push_back(new_attrdesc(4, VK_FORMAT_R32_SFLOAT, &vertex::border_width));
            attribute_descriptions.push_back(new_attrdesc(5, VK_FORMAT_R32G32B32A32_SFLOAT, &vertex::border_color));
            attribute_descriptions.push_back(new_attrdesc(6, VK_FORMAT_R32G32_SFLOAT, &vertex::center));
            attribute_descriptions.push_back(new_attrdesc(7, VK_FORMAT_R32G32_SFLOAT, &vertex::uv));
            attribute_descriptions.push_back(new_attrdesc(8, VK_FORMAT_R32_UINT, &vertex::tex_index));

            return attribute_descriptions;
        }
    };

    struct push_constant {
        glm::mat4 projection;
    };
}
