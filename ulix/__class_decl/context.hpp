#pragma once

#include "../object.hpp"
#include "../file.hpp"
#include "../font.hpp"
#include "../pixmap.hpp"
#include "../appinfo.hpp"
#include "../renderinfo.hpp"
#include "../wininfo.hpp"
#include "../__inside_impl/vulkan_classes.hpp"
#include "../__inside_impl/vulkan_shader_data_classes.hpp"
#include "../scene.hpp"
#include "../cursor.hpp"


template<> struct std::hash<ulx::pair<ulx::file, ulx::recsize>> {
    ulx::u64 operator()(const pair<ulx::file, ulx::recsize>& p) const noexcept {
        ulx::u64 h = std::hash<ulx::file>{}(p.first);
        h ^= std::hash<ulx::recsize>{}(p.second) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};
template<> struct std::hash<ulx::pair<std::string, ulx::font>> {
    ulx::u64 operator()(const pair<std::string, ulx::font>& p) const noexcept {
        ulx::u64 h = std::hash<std::string>{}(p.first);
        h ^= std::hash<ulx::font>{}(p.second) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

namespace ulx {
    class context {
        /* texture caches */
        private:
            hashmap<file, u32> bitmap_texture_cache;
            hashmap<pair<file, recsize>, u32> vector_texture_cache;
            hashmap<pair<str, font>, u32> font_texture_cache;
        
        /* tick callback & configs */
        public:
            using tick_callback = callback<void(context&, u64)>;
        private:
            time<steady> last_time;
            tick_callback ticker;
            u64 tick_delay = 0;

        /* render callback & vulkan datas & scene datas */
        public:
            using render_callback = renderinfo::render_callback;
        private:
            render_callback renderer;

            vec<__uii::vsdces::vertex> render_vertices;
            vec<u32> render_indices;
            __uii::vsdces::push_constant push_const;

            scene current_render_scene;
            color current_background_color;
            
            bool first_render = true, first_frame = true, swapchain_dirty = false;
            cursor curs = cursor::resetcur;
        private:
            auto update_push_constant() -> void;
            auto update_scene_objects() -> void;
            auto push_objects(vec<object>& objects, u32& render_index, const recsize& parent_size, 
                const recpos& parent_pos, box parent_layout, f32& x_offset, f32& y_offset, 
                f32 parent_padding, u8 align) -> void;
            auto push_object(const object& object, const recsize& siz, const recpos& pos, 
                u32& render_index, u32 texture_index) -> void;
            auto set_layout(recsize& siz, recpos& pos, const recsize& parent_size, const recpos& parent_pos, 
                box parent_layout, f32& parent_x_offset, f32& parent_y_offset, 
                f32 parent_padding, u8 align) -> void;
            auto event_valid(
                object::pressed_event pressed_event, 
                object::released_event released_event, 
                object::hovered_event hovered_event) -> bool;

        /* window handle & window configs & native window states */ 
        private:
            UINT window_dpi; u32 window_native_titlebar_height;
            
            HWND window_hwnd = nullptr, snap_trigger_hwnd = nullptr;
            bool window_titled = false, window_resizable = false;
            u32 window_width = 0, window_height = 0;
            recsize window_min_size, window_max_size;
            
            u32 vtitlebar_height, vclosebtn_width, vminbtn_width, vmaxbtn_width;
            bool window_max = false, window_min = false, window_restore = false;
        private:
            auto create_window(const wininfo& window_info, const appinfo& app_info) -> void;

        /* window interfaces */
        public:
            auto dpi() const -> UINT;
            auto exec() -> int;
            auto show() const -> void;
            auto hide() const -> void;
            auto exit() const -> void;
            auto size() const -> recsize;
            auto pos() const -> recpos;
            auto width() const -> f32;
            auto height() const -> f32;
            auto x() const -> f32;
            auto y() const -> f32;
            auto resize(const recsize& size) -> void;
            auto mov(const recpos& pos) -> void;
            auto set_min(const recsize& size) -> void;
            auto set_max(const recsize& size) -> void;
            auto set_tick(const tick_callback& callback) -> void;
            auto set_delay(u64 delay) -> void;
            auto set_cur(cursor cur) -> void;
            auto cur_pos() const -> recpos;
            auto pressing() const -> bool;
            auto max() -> void;
            auto min() -> void;
            auto restore() -> void;
            auto is_maxed() const -> bool;

        /* vulkan datas & application states */
        private:
            VkInstance vulkan_instance;
            VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
            VkPhysicalDevice physical_device = VK_NULL_HANDLE;
            VkDevice logical_device;
            VkSurfaceKHR window_surface;
            VkSwapchainKHR swapchain;
            VkFormat swapchain_image_format;
            VkExtent2D swapchain_extent;
            VkRenderPass render_pass;
            VkPipelineLayout pipeline_layout;
            VkPipeline graphics_pipeline;
            VkCommandPool command_pool;
            VkDescriptorSetLayout descriptor_set_layout;
            VkDeviceMemory texture_image_memory;
            VkSampler texture_image_sampler;
            VkDescriptorUpdateTemplate descriptor_update_template = VK_NULL_HANDLE;
            VkDeviceSize last_vertex_buffer_size = 0, last_index_buffer_size = 0, last_texture_buffer_size = 0;
            
            __uii::vkclses::buffer texture_buffer;
            __uii::vkclses::buffer vertex_buffer;
            __uii::vkclses::buffer index_buffer;
            __uii::vkclses::physical_device_info physical_device_infos;
            __uii::vkclses::device_queues device_queues;
            
            vec<pixmap> texture_image_pixmaps;
            vec<VkDescriptorPool> descriptor_pools;
            vec<VkDescriptorSet> descriptor_sets;
            vec<__uii::vkclses::texture_image> texture_images;
            vec<VkCommandBuffer> command_buffers;
            vec<VkSemaphore> image_available_semaphores;
            vec<VkFence> in_flight_fences;
            vec<VkFramebuffer> swapchain_frame_buffers;
            vec<VkImage> swapchain_images;
            vec<VkImageView> swapchain_image_views;
            vec<vec<VkFramebuffer>> pending_framebuffers;
            vec<vec<VkImageView>> pending_image_views;
            vec<VkSwapchainKHR> pending_swapchains;
            vec<VkSemaphore> render_finished_semaphores;
            vec<vec<VkSemaphore>> pending_semaphores;

            pathstr cache_file_path;
            u32 max_texture_count;
            stablestr<> application_id;
            u32 application_version;
            bool need_to_push_textures = false;
            u32 max_frames_in_flight, current_frame = 0;
            present_mode pmode;
        private:
            auto render() -> void;
            auto create_vulkan_objects(const appinfo& application_info, const renderinfo& render_info) -> void;
            auto destroy_vulkan_objects() -> void;
            auto record_command_buffer(VkCommandBuffer command_buffer, u32 image_index) -> void;
            auto create_vulkan_instance() -> void;
            auto check_validation_layer_support() -> bool;
            auto create_debug_messenger() -> void;
            auto destroy_debug_utils_messenger() -> void;
            auto populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) -> void;
            auto select_physical_device() -> void;
            auto create_logical_device() -> void;
            auto create_window_surface() -> void;
            auto create_swapchain(VkSwapchainKHR swapchain = VK_NULL_HANDLE) -> void;
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
            auto create_texture_buffer() -> void;
            auto create_vertex_buffer() -> void;
            auto create_index_buffer() -> void;
            auto update_object_buffers() -> void;
            auto create_descriptor_set_layout() -> void;
            auto create_descriptor_pools() -> void;
            auto map_texture_images() -> void;
            auto create_texture_image(u64 index) -> VkMemoryRequirements;
            auto create_texture_image_views() -> void;
            auto create_texture_image_sampler() -> void;
            auto push_texture_image(const pixmap& pixmap) -> void;
            auto push_texture_images() -> void;
            auto recreate_texture_buffer() -> void;
            auto create_texture_images() -> VkDeviceSize;
            auto update_texture_images(recsize& siz, const object& object, u32& texture_index) -> void;
            auto create_descriptor_update_template() -> void;
            auto update_texture_descriptors() -> void;
            auto create_render_finished_semaphores() -> void;

        private:
            static auto WINAPI win32_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT;
            static auto CALLBACK snap_trigger_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT;
            static VKAPI_ATTR auto VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] auto* user_data) -> VkBool32;
            
        public:
            inline constexpr context() = default;
            constexpr context(const appinfo& application_info, const wininfo& window_info, const renderinfo& render_info);
            ~context();
    };
}
