#pragma once
#include "../element.hpp"
#include "../pixmap.hpp"
#include "../appinfo.hpp"
#include "../renderinfo.hpp"
#include "../wininfo.hpp"
#include "../__inside_impl/vulkan_classes.hpp"
#include "../__inside_impl/vulkan_shader_data_classes.hpp"
#include "../__inside_impl/atlas_texture_merger.hpp"
#include "../scene.hpp"
#include "../cursor.hpp"
#include "ulix/path.hpp"
#include "ulix/types.hpp"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace ulx {
    template<typename t_dxobj> using dxptr = Microsoft::WRL::ComPtr<t_dxobj>;
    
    class context {
        // texture caches
        private:
            hashmap<element::texture::bitmap, __uii::ats::idx> m_bitmap_cache;
            hashmap<element::texture::vector, __uii::ats::idx> m_vector_cache;
            hashmap<element::texture::text, __uii::ats::idx>   m_font_cache;

            u32 m_atlas_x = 0, 
                m_atlas_y = 0;
            u32 m_atlas_index = 1;
            u32 m_atlas_height = 0;

            auto save_texture_caches() -> void;
        
        // tick callback & configs
        public: using tickcallback = callback<void(context&, u64)>;
        private:
            time<steady> m_lasttime      = steady::now();
            tickcallback m_tickcallback;
            u64          m_delay         = 0;

        // render callback & vulkan datas & scene datas
        public: using rendercallback = renderinfo::render_callback;
        private:
            rendercallback m_rendercallback;

            vec<__uii::vsdces::vertex>  m_vertices;
            vec<u32>                    m_indices;
            __uii::vsdces::pushconstant m_pushconstant;

            scene m_scene;
            color m_background_color;
            cursor m_cursor = cursor::resetcur;
            
            bool m_first = true, 
                 m_dirty = false;
        private:
            auto update_push_constant() -> void;
            auto update_scene_objects() -> void;
            
            auto push_elements(
                vec<element>& elements, u32& render_index, 
                const size& parent_size, const point& parent_pos, 
                box parent_layout, f32 parent_padding, 
                f32& x_offset, f32& y_offset, u8 align, 
                POINT& cursor_pt, HWND pt_hwnd, int left_mouse_state) -> void;
            auto push_element(
                const element& elem, 
                const size& siz, const point& pos, 
                u32& render_index, u32 texture_index, 
                __uii::ats::uv& uv) -> void;
            auto compute_layout(
                size& siz, point& pos, 
                const size& parent_size, const point& parent_pos, 
                box parent_layout, 
                f32& x_offset, f32& y_offset, 
                f32 padding, u8 align) -> void;
            auto event_valid(
                const element::event::pressed& pressed_event, 
                const element::event::released& released_event, 
                const element::event::hovered& hovered_event) -> bool;
            auto read_cache() -> void;

        // window handle & window configs & native window states 
        private:
            UINT m_mainwin_dpi; u32 m_native_titleh;
            
            HWND m_mainwin = nullptr, 
                 m_snapwin = nullptr;
            bool m_mainwin_titled = false, 
                 m_mainwin_resizable = false;
            u32  m_mainwin_width = 0, 
                 m_mainwin_height = 0;
            size m_mainwin_minsize, 
                 m_mainwin_maxsize;
            
            u32  m_vtitlebar_height, 
                 m_vclosebtn_width, 
                 m_vminbtn_width, 
                 m_vmaxbtn_width;
            bool m_maxtag = false, 
                 m_mintag = false, 
                 m_restoretag = false;
                 
            auto create_window(const wininfo& window_info, const appinfo& app_info) -> void;

        // window interfaces
        public:
            auto exec() -> int;
            auto exit() const -> void;
            auto set_tick(const tickcallback& callback, u64 delay) -> void;
            
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
            auto win_max() -> void;
            auto win_min() -> void;
            auto win_restore() -> void;

        // vulkan objects & application states
        private:
            VkInstance                 m_vulkan_instance;
            VkDebugUtilsMessengerEXT   m_debug_messenger = VK_NULL_HANDLE;
            VkPhysicalDevice           m_physical_device = VK_NULL_HANDLE;
            VkDevice                   m_logical_device;
            VkSurfaceKHR               m_window_surface;
            VkRenderPass               m_render_pass;
            VkPipelineLayout           m_pipeline_layout;
            VkPipeline                 m_graphics_pipeline;
            VkCommandPool              m_command_pool;
            VkDescriptorSetLayout      m_descriptor_set_layout;
            VkDeviceMemory             m_texture_image_memory;
            VkSampler                  m_texture_image_sampler;
            VkDescriptorUpdateTemplate m_descriptor_update_template = VK_NULL_HANDLE;
            VkDeviceSize               m_vertex_size = 0, 
                                       m_index_size = 0, 
                                       m_texture_size = 0;
            VkDescriptorPool           m_descriptor_pool;
            VkDeviceMemory             m_shared_memory      = VK_NULL_HANDLE;
            VkFramebuffer              m_shared_framebuffer = VK_NULL_HANDLE;
            HANDLE                     m_shared_handle      = nullptr;

            
            __uii::vkclses::buffer               m_texture_buffer,
                                                 m_vertex_buffer,
                                                 m_index_buffer;
            __uii::vkclses::physical_device_info m_physical_device_infos;
            __uii::vkclses::device_queues        m_device_queues;
            __uii::vkclses::image                m_shared_image;

            
            vec<pixmap>                        m_texture_image_pixmaps;
            vec<VkDescriptorSet>               m_descriptor_sets;
            vec<__uii::vkclses::image>         m_texture_images;
            vec<VkCommandBuffer>               m_command_buffers;
            vec<VkSemaphore>                   m_image_available_semaphores;
            vec<VkFence>                       m_in_flight_fences;
            vec<VkSemaphore>                   m_render_finished_semaphores;
            vec<vec<VkSemaphore>>              m_pending_semaphores;
            vec<VkDescriptorImageInfo>         m_descriptor_image_infos;

            path         m_cache_path;
            stablestr<>  m_appid;
            u32          m_appversion;
            bool         m_texture_pushtag = true;
            u32          m_max_frames = 2,
                         m_current_frame = 0;
            
            auto render_frame() -> void;
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
            auto init_descriptor_image_infos() -> void;
            auto create_shared_image() -> void;
            auto allocate_shared_image_memory() -> void;
            auto create_shared_image_view() -> void;
            auto create_shared_framebuffer() -> void;


        // DXGI objects
        private:
            ID3D11Device*           m_d3d_device             = nullptr;
            ID3D11DeviceContext*    m_d3d_device_context     = nullptr;
            IDXGISwapChain1*        m_swapchain              = nullptr;
            ID3D11RenderTargetView* m_render_target_view     = nullptr;
            ID3D11Texture2D*        m_shared_texture         = nullptr;
            ID3D11RenderTargetView* m_shared_render_target_view         = nullptr;
        
            auto create_d3d_objects() -> void;
            auto create_render_target_view() -> void;
            auto create_swapchain() -> void;
            auto check_shared_image_support() -> void;
            auto resize_swapchain() -> void;
            auto create_shared_texture() -> void;

        private:
            static auto WINAPI win32_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT;
            static auto CALLBACK snapwin_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT;
            static VKAPI_ATTR auto VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] auto* user_data) -> VkBool32;
            
        public:
            inline constexpr context() = default;
            constexpr context(const appinfo& application_info, const wininfo& window_info, const renderinfo& render_info);
            ~context();
    };
}
