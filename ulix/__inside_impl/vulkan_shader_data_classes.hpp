#pragma once

#include <vulkan/vulkan_core.h>
#include "../types.hpp"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace __uii::vsdces {
    struct Vertex2D {
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
            binding_description.stride = sizeof(Vertex2D);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return binding_description;
        }

        template<typename M>
        inline static auto new_attrdesc(ulx::u32 location, VkFormat format, M Vertex2D::*member) -> VkVertexInputAttributeDescription {
            VkVertexInputAttributeDescription vertex_input_attribute_description{};
            vertex_input_attribute_description.location = location;
            vertex_input_attribute_description.binding = 0;
            vertex_input_attribute_description.format = format;
            vertex_input_attribute_description.offset = static_cast<ulx::u32>(reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<Vertex2D const volatile*>(0)->*member)));

            return vertex_input_attribute_description;
        }

        inline static auto get_attribute_descriptions() -> ulx::vec<VkVertexInputAttributeDescription> {
            ulx::vec<VkVertexInputAttributeDescription> attribute_descriptions;
            attribute_descriptions.push_back(new_attrdesc(0, VK_FORMAT_R32G32_SFLOAT, &Vertex2D::position));
            attribute_descriptions.push_back(new_attrdesc(1, VK_FORMAT_R32G32B32A32_SFLOAT, &Vertex2D::color));
            attribute_descriptions.push_back(new_attrdesc(2, VK_FORMAT_R32_SFLOAT, &Vertex2D::corner_radius));
            attribute_descriptions.push_back(new_attrdesc(3, VK_FORMAT_R32G32_SFLOAT, &Vertex2D::half_size));
            attribute_descriptions.push_back(new_attrdesc(4, VK_FORMAT_R32_SFLOAT, &Vertex2D::border_width));
            attribute_descriptions.push_back(new_attrdesc(5, VK_FORMAT_R32G32B32A32_SFLOAT, &Vertex2D::border_color));
            attribute_descriptions.push_back(new_attrdesc(6, VK_FORMAT_R32G32_SFLOAT, &Vertex2D::center));
            attribute_descriptions.push_back(new_attrdesc(7, VK_FORMAT_R32G32_SFLOAT, &Vertex2D::uv));
            attribute_descriptions.push_back(new_attrdesc(8, VK_FORMAT_R32_UINT, &Vertex2D::tex_index));

            return attribute_descriptions;
        }
    };

    struct PushConstant {
        glm::mat4 projection;
    };
}
