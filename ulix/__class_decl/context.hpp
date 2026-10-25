#pragma once
#include "../element.hpp"
#include "../pixmap.hpp"
#include "../appinfo.hpp"
#include "../renderinfo.hpp"
#include "../wininfo.hpp"
#include "../__inside_impl/vulkan_classes.hpp"
#include "../__inside_impl/vulkan_shader_data_classes.hpp"
#include "../__inside_impl/atlaspusher.hpp"
#include "../scene.hpp"
#include "../cursor.hpp"
#include <future>
#include <mutex>

namespace ulx {
    class context {
        /* texture caches */
        private:
            hashmap<element::texture::bitmap, __uii::ats::idx> bitmap_texture_cache;
            hashmap<element::texture::vector, __uii::ats::idx> vector_texture_cache;
            hashmap<element::texture::text, __uii::ats::idx> font_texture_cache;
            
            u32 current_atlas_index = 1;
            u32 current_atlas_x = 0, current_atlas_y = 0;
            u32 current_atlas_max_height = 0;
        
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
            auto push_elements(vec<element>& elements, u32& render_index, const size& parent_size, 
                const point& parent_pos, box parent_layout, f32& x_offset, f32& y_offset, 
                f32 parent_padding, u8 align) -> void;
            auto push_element(const element& object, const size& siz, const point& pos, 
                u32& render_index, u32 texture_index, __uii::ats::uv& uv) -> void;
            auto set_layout(size& siz, point& pos, const size& parent_size, const point& parent_pos, 
                box parent_layout, f32& parent_x_offset, f32& parent_y_offset, 
                f32 parent_padding, u8 align) -> void;
            auto event_valid(
                const element::event::pressed& pressed_event, 
                const element::event::released& released_event, 
                const element::event::hovered& hovered_event) -> bool;

        /* window handle & window configs & native window states */ 
        private:
            UINT window_dpi; u32 window_native_titlebar_height;
            
            HWND window_hwnd = nullptr, snap_trigger_hwnd = nullptr;
            bool window_titled = false, window_resizable = false;
            u32 window_width = 0, window_height = 0;
            size window_min_size, window_max_size;
            
            u32 vtitlebar_height, vclosebtn_width, vminbtn_width, vmaxbtn_width;
            bool window_max = false, window_min = false, window_restore = false;
        private:
            auto create_window(const wininfo& window_info, const appinfo& app_info) -> void;

        /* window interfaces */
        public:
            auto exec() -> int;
            auto exit() const -> void;
            auto set_tick(const tick_callback& callback) -> void;
            auto set_delay(u64 delay) -> void;
            
            auto win_dpi() const -> UINT;
            auto win_show() const -> void;
            auto win_hide() const -> void;
            auto win_size() const -> ulx::size;
            auto win_pos() const -> point;
            auto win_width() const -> f32;
            auto win_height() const -> f32;
            auto win_x() const -> f32;
            auto win_y() const -> f32;
            auto win_cursor() const -> point;
            auto win_maxed() const -> bool;
            auto win_mined() const -> bool;
            
            auto win_resize(const ulx::size& size) -> void;
            auto win_move(const point& pos) -> void;
            auto win_set_minsize(const ulx::size& size) -> void;
            auto win_set_maxsize(const ulx::size& size) -> void;
            auto win_set_cursor(cursor cur) -> void;
            auto win_max() -> void;
            auto win_min() -> void;
            auto win_restore() -> void;

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

            vec<std::future<void>> pending_fns;
            std::mutex pending_mutex;

            pathstr cache_file_path;
            u32 max_texture_count;
            stablestr<> application_id;
            u32 application_version;
            bool need_to_push_textures = true;
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
            auto update_texture_images(ulx::size& siz, const element& object, u32& texture_index) -> __uii::ats::uv;
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
