#pragma once

#include "../object.hpp"
#include "../__require_libraries/skarupke/flat_hash_map.hpp"
#include "../align.hpp"
#include "../file.hpp"
#include "../font.hpp"
#include "../pixmap.hpp"
#include "../rect.hpp"
#include "../appinfo.hpp"
#include "../renderinfo.hpp"
#include "../wininfo.hpp"
#include "../__inside_impl/vulkan_classes.hpp"
#include "../__inside_impl/vulkan_shader_data_classes.hpp"
#include "../scene.hpp"
#include "../timer.hpp"


template<> struct std::hash<std::pair<ulx::file, ulx::rect>> {
    size_t operator()(const std::pair<ulx::file, ulx::rect>& p) const noexcept {
        size_t h = std::hash<ulx::file>{}(p.first);
        h ^= std::hash<ulx::rect>{}(p.second) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};
template<> struct std::hash<std::pair<std::string, ulx::font>> {
    size_t operator()(const std::pair<std::string, ulx::font>& p) const noexcept {
        size_t h = std::hash<std::string>{}(p.first);
        h ^= std::hash<ulx::font>{}(p.second) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

class UContext {
    public:
        using RenderCallback = ulx::scene(*)(UContext&);
        using TickTimer = ulx::timer<UContext&>;

    private:
        TickTimer tick_timer;
        RenderCallback render_callback;
        ulx::scene current_render_scene;
        ulx::vec<__uii::vsdces::Vertex2D> render_vertices;
        ulx::vec<ulx::u32> render_indices;
        __uii::vsdces::PushConstant push_constant;
        ulx::color current_background_color;
        bool render_dirty = false;
        ulx::u32 texture_index_max;
        bool data_dirty = false;
        bool rending = false;
        bool first_frame = true;

        ska::flat_hash_map<ulx::file, ulx::u32> bitmap_texture_cache;
        ska::flat_hash_map<ulx::pair<ulx::file, ulx::rect>, ulx::u32> vector_texture_cache;
        ska::flat_hash_map<ulx::pair<ulx::str, ulx::font>, ulx::u32> font_texture_cache;

        auto update_push_constant() -> void;
        auto update_scene_objects() -> void;
        auto push_objects(const ulx::vec<ulx::object>& objects, ulx::u32& render_index, ulx::f32 window_height, const ulx::rect& parent_rect) -> void;

    public:
        auto set_timer(const TickTimer& timer) -> void;
        auto dirtied(bool data_dirty = false) -> void;

    private:
        UINT window_dpi;
        HWND window_hwnd = nullptr;
        bool window_titled_state = false;
        bool window_resizable_state = false;
        ulx::u32 window_current_width, window_current_height;
        ulx::rect window_min_rect, window_max_rect;
        ulx::u32 window_titlebar_height;
        bool was_maximized = false;

        auto create_window(const ulx::wininfo& window_info) -> void;

    public:
        auto titled() const -> bool;
        auto resizable() const -> bool;
        auto dpi() const -> UINT;
        auto exec() -> int;
        auto show() -> void;
        auto hide() -> void;
        auto exit() -> void;
        auto rect() -> ulx::rect;
        auto set_rect(const ulx::rect& rect) -> void;
        auto min_rect() const -> ulx::rect;
        auto set_min_rect(const ulx::rect& rect) -> void;
        auto max_rect() const -> ulx::rect;
        auto set_max_rect(const ulx::rect& rect) -> void;
        auto rendering() const -> bool;
        static auto init() -> void;

    private:
        VkInstance vulkan_instance;
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        __uii::vkclses::PhysicalDeviceInfos physical_device_infos;
        VkDevice logical_device;
        __uii::vkclses::DeviceQueues device_queues;
        VkSurfaceKHR window_surface;
        VkSwapchainKHR swapchain;
        ulx::vec<VkImage> swapchain_images;
        ulx::vec<VkImageView> swapchain_image_views;
        VkFormat swapchain_image_format;
        VkExtent2D swapchain_extent;
        VkRenderPass render_pass;
        VkPipelineLayout pipeline_layout;
        VkPipeline graphics_pipeline;
        ulx::vec<VkFramebuffer> swapchain_frame_buffers;
        VkCommandPool command_pool;
        ulx::vec<VkCommandBuffer> command_buffers;
        ulx::vec<VkSemaphore> image_available_semaphores;
        ulx::vec<VkSemaphore> render_finished_semaphores;
        ulx::vec<VkFence> in_flight_fences;
        ulx::size max_frames_in_flight;
        ulx::u32 current_frame = 0;
        __uii::vkclses::Buffer staging_buffer;
        __uii::vkclses::Buffer vertex_buffer;
        __uii::vkclses::Buffer index_buffer;
        VkDescriptorSetLayout descriptor_set_layout;
        ulx::vec<ulx::pixmap> texture_image_pixmaps;
        bool enabled_debug = false;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;
        ulx::vec<__uii::vkclses::TextureImage> texture_images;
        VkDeviceMemory texture_image_memory;
        VkSampler texture_image_sampler;
        ulx::str cache_file_path;
        ulx::u32 max_texture_count;
        ulx::str application_id;
        ulx::u32 application_version;
        ulx::u64 staging_buffer_size;
        bool push_texture_images_to_gpu = false;

        auto create_vulkan_objects(const ulx::appinfo& application_info, const ulx::renderinfo& render_info) -> void;
        auto destroy_vulkan_objects() -> void;
        auto record_command_buffer(VkCommandBuffer command_buffer, ulx::u32 image_index) -> void;
        auto draw_frame() -> void;
        auto create_vulkan_instance() -> void;
        auto check_validation_layer_support() -> bool;
        auto create_debug_messenger() -> void;
        auto destroy_debug_utils_messenger() -> void;
        auto populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) -> void;
        auto select_physical_device() -> void;
        auto create_logical_device() -> void;
        auto create_window_surface() -> void;
        auto create_swapchain() -> void;
        auto destroy_swapchain() -> void;
        auto recreate_swapchain() -> void;
        auto get_swapchain_images() -> void;
        auto create_swapchain_image_views() -> void;
        auto create_shader_module(unsigned char* code, unsigned int length) -> VkShaderModule;
        auto populate_pipeline_shader_stage_create_info(VkPipelineShaderStageCreateInfo& create_info, VkShaderModule shader_module, VkShaderStageFlagBits stage) -> void;
        auto create_graphics_pipeline() -> void;
        auto create_render_pass() -> void;
        auto create_swapchain_frame_buffers() -> void;
        auto create_command_pool() -> void;
        auto create_command_buffers() -> void;
        auto create_sync_objects() -> void;
        auto create_staging_buffer() -> void;
        auto create_vertex_buffer() -> void;
        auto create_index_buffer() -> void;
        auto update_object_buffers() -> void;
        auto create_descriptor_set_layout() -> void;
        auto create_descriptor_pool() -> void;
        auto create_descriptor_set() -> void;
        auto map_texture_images() -> void;
        auto create_texture_image(ulx::size index) -> VkMemoryRequirements;
        auto create_texture_image_views() -> void;
        auto create_texture_image_sampler() -> void;
        auto push_texture_image_cpu(const ulx::pixmap& pixmap) -> void;
        auto push_texture_images_gpu() -> void;
        auto recreate_staging_buffer() -> void;
        auto create_texture_images() -> VkDeviceSize;
        auto update_texture_images(ulx::rect& rect, const ulx::object& object, ulx::u32& texture_index) -> void;
        auto push_object(const ulx::object& object, ulx::rect rect, ulx::u32& render_index, ulx::f32 window_height, ulx::u32 texture_index) -> void;

    private:
        static auto WINAPI window_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT;
        static VKAPI_ATTR auto VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] auto* user_data) -> VkBool32;

    public:
        inline UContext() = default;
        UContext(const ulx::appinfo& application_info, const ulx::wininfo& window_info, const ulx::renderinfo& render_info);
        ~UContext();
};
