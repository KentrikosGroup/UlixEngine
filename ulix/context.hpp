#pragma once

#include <algorithm>
#include <concepts>
#include <execution>
#include <variant>
#include <vulkan/vulkan_core.h>
#include "align.hpp"
#include "file.hpp"
#include "glm/ext/vector_float4.hpp"
#include "layout.hpp"
#include "pixmap.hpp"
#include "recsize.hpp"
#include "renderinfo.hpp"
#include "timer.hpp"
#include "scene.hpp"
#include "__inside_impl/vulkan_classes.hpp"
#include "__inside_impl/vulkan_shader_data_classes.hpp"
#include "__inside_impl/vulkan_algorithm.hpp"
#include "__inside_impl/vulkan_requirements.hpp"
#include "__class_decl/context.hpp"
#include "rect.hpp"
#include "log.hpp"
#include "color.hpp"
#include "object.hpp"
#include "wininfo.hpp"
#include "appinfo.hpp"
#include "winfunc.hpp"
#include "types.hpp"
#include <cstdint>
#include <synchapi.h>
#include <windef.h>
#include <winuser.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <wincodec.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ranges>

#include "__builtin_shaders/fragment_shader.inl"
#include "__builtin_shaders/vertex_shader.inl"



inline auto ulx::context::update_push_constant() -> void {
    push_constant.projection = glm::ortho(
        0.0f, static_cast<ulx::f32>(window_current_width),
        static_cast<ulx::f32>(window_current_height), 0.0f,
        -1.0f, 1.0f);
}

inline auto ulx::context::update_texture_images(ulx::recsize& siz, const ulx::object& object, ulx::u32& texture_index) -> void {
    ulx::object::texture_variant texture_variant = object.get_texture();
    
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::same_as<T, std::monostate>) {
            texture_index = 0;
        } else if constexpr (std::same_as<T, ulx::file>) {
            auto it = bitmap_texture_cache.find(arg);
            if (it != bitmap_texture_cache.end())
                texture_index = it->second;
            else {
                push_texture_image_cpu(ulx::pixmap::from_bitmap(arg));
                texture_index = texture_image_pixmaps.size() - 1;
                bitmap_texture_cache[arg] = texture_index;
            }
    
            ulx::pixmap pixmap = texture_image_pixmaps[texture_index];
            ulx::recsize pixrec = pixmap.get_size();
            if (siz.width() == -1) siz = ulx::recsize(pixrec.width(), siz.height());
            if (siz.height() == -1) siz = ulx::recsize(siz.width(), pixrec.height());
        } else if constexpr (std::same_as<T, ulx::pair<ulx::file, ulx::recsize>>) {
            auto it = vector_texture_cache.find(arg);
            if (it != vector_texture_cache.end())
                texture_index = it->second;
            else {
                ulx::f32 width = arg.second.width();
                ulx::f32 height = arg.second.height();
                ulx::f32 physical_width = ulx::wfn::physical_cast(width, window_dpi);
                ulx::f32 physical_height = ulx::wfn::physical_cast(height, window_dpi);
    
                push_texture_image_cpu(ulx::pixmap::from_vector(arg.first, physical_width, physical_height));
                texture_index = texture_image_pixmaps.size() - 1;
                vector_texture_cache[arg] = texture_index;
            }
    
            ulx::pixmap pixmap = texture_image_pixmaps[texture_index];
            ulx::recsize pixrec = pixmap.get_size();
            if (siz.width() == -1) siz = ulx::recsize(pixrec.width() / 2, siz.height());
            if (siz.height() == -1) siz = ulx::recsize(siz.width(), pixrec.height() / 2);
        } else if constexpr (std::same_as<T, ulx::pair<ulx::str, ulx::font>>) {
            auto it = font_texture_cache.find(arg);
            if (it != font_texture_cache.end())
                texture_index = it->second;
            else {
                push_texture_image_cpu(ulx::pixmap::from_text(arg.first, arg.second));
                texture_index = texture_image_pixmaps.size() - 1;
                font_texture_cache[arg] = texture_index;
            }
    
            ulx::pixmap pixmap = texture_image_pixmaps[texture_index];
            ulx::recsize pixrec = pixmap.get_size();
            ulx::font font = arg.second;
            if (siz.width() == -1) siz = ulx::recsize(pixrec.width() / font.get_scale(), siz.height());
            if (siz.height() == -1) siz = ulx::recsize(siz.width(), pixrec.height() / font.get_scale());
        }
    }, texture_variant);
    siz = ulx::recsize(siz.width() * object.get_scale(), siz.height() * object.get_scale());
}

inline auto ulx::context::push_object(const ulx::object& object, const ulx::recsize& siz, const ulx::recpos& pos, ulx::u32& render_index, ulx::f32 window_height, ulx::u32 texture_index) -> void {
    ulx::color background_color = object.get_background_color();
    ulx::color border_color = object.get_border_color();
    float corner_radius = ulx::wfn::physical_cast(object.get_corner_radius(), window_dpi);
    float border_width = ulx::wfn::physical_cast(object.get_border_width(), window_dpi);
    float x = ulx::wfn::physical_cast(pos.x(), window_dpi);
    float y = ulx::wfn::physical_cast(pos.y(), window_dpi);
    float width = ulx::wfn::physical_cast(siz.width(), window_dpi);
    float height = ulx::wfn::physical_cast(siz.height(), window_dpi);
    glm::vec2 half_size = {width / 2, height / 2};
    glm::vec2 center = {x + half_size.x + 0.5f, window_height - (y + half_size.y) + 0.5f};
    glm::vec2 top_left = center - half_size;
    glm::vec2 top_right = {center.x + half_size.x, center.y - half_size.y};
    glm::vec2 bottom_right = center + half_size;
    glm::vec2 bottom_left = {center.x - half_size.x, center.y + half_size.y};
    glm::vec4 background_color_vec = __uii::vkalg::color_to_vec4(background_color);
    glm::vec4 border_color_vec = __uii::vkalg::color_to_vec4(border_color);

    render_vertices.insert(render_vertices.end(), {
        {top_right, background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {1.0f, 1.0f}, texture_index},
        {top_left, background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {0.0f, 1.0f}, texture_index},
        {bottom_left, background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {0.0f, 0.0f}, texture_index},
        {bottom_right, background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {1.0f, 0.0f}, texture_index}});
    render_indices.insert(render_indices.end(), {
        render_index, render_index + 1, render_index + 2,
        render_index + 2, render_index + 3, render_index});
    render_index += 4;
}

inline auto ulx::context::set_layout(ulx::recsize& siz, ulx::recpos& pos, const ulx::recsize& parent_size, const ulx::recpos& parent_pos, ulx::layout parent_layout, ulx::f32& pxoff, ulx::f32& pyoff, ulx::f32 ppad, ulx::u8 alignment) -> void {
    if (parent_layout == ulx::nonebox) return;

    if (parent_layout == ulx::horbox) {
        if (alignment & ulx::align::alignleft) {
            pos = ulx::recpos(pxoff + pos.x(), pos.y());
            pxoff += ppad + siz.width();
        } else if (alignment & ulx::align::alignright) {
            pos = ulx::recpos(parent_size.width() - pxoff - siz.width(), pos.y());
            pxoff += ppad + siz.width();
        }

        if (alignment & ulx::align::alignbottom) {
            pos = ulx::recpos(pos.x(), parent_size.height() + pos.y() - siz.height());
        }
    }

    if (parent_layout == ulx::verbox) {
        if (alignment & ulx::aligntop) {
            pos = ulx::recpos(pos.x(), pyoff + pos.y());
            pyoff += ppad + siz.height();
        } else if (alignment & ulx::alignbottom) {
            pos = ulx::recpos(pos.x(), parent_size.height() - pyoff - siz.height());
            pyoff += ppad + siz.height();
        }
    
        if (alignment & ulx::alignright) {
            pos = ulx::recpos(parent_size.width() + pos.x() - siz.width(), pos.y());
        }
    }
}

inline auto ulx::context::push_objects(const ulx::vec<ulx::object>& objects, ulx::u32& render_index, ulx::f32 window_height, const ulx::recsize& parent_size, const ulx::recpos& parent_pos, ulx::layout parent_layout, ulx::f32& pxoff, ulx::f32& pyoff, ulx::f32 ppad, ulx::u8 alignment) -> void {
    if (objects.empty()) return;
    for (auto& object : objects) {
        ulx::recsize siz = object.get_size(); 
        ulx::recpos pos = object.get_pos();
        ulx::u32 texture_index;
        update_texture_images(siz, object, texture_index);
        if (siz.width() == -2) siz = ulx::recsize(parent_size.width(), siz.height());
        if (siz.height() == -2) siz = ulx::recsize(siz.width(), parent_size.height());
        pos = ulx::recpos(pos.x() + parent_pos.x(), pos.y() + parent_pos.y());
        set_layout(siz, pos, parent_size, parent_pos, parent_layout, pxoff, pyoff, ppad, alignment);
        push_object(object, siz, pos, render_index, window_height, texture_index);

        ulx::vec<ulx::object> child_objects = object.get_objects();
        ulx::f32 mpxoff = 0; ulx::f32 mpyoff = 0;
        push_objects(child_objects, render_index, window_height, siz, pos, object.get_layout(), mpxoff, mpyoff, object.get_padding(), object.get_alignment());
    }
}

inline auto ulx::context::update_scene_objects() -> void {
    ulx::u32 render_index = 0;
    render_vertices.clear(); render_indices.clear();
    current_render_scene = renderer(*this);
    ulx::vec<ulx::object> objects = current_render_scene.get_objects();
    current_background_color = current_render_scene.get_background_color();

    ulx::f32 pxoff = 0, pyoff = 0;
    push_objects(objects, render_index, static_cast<ulx::f32>(window_current_height),
        ulx::recsize(ulx::wfn::logical_cast(window_current_width, window_dpi),
                    ulx::wfn::logical_cast(window_current_height, window_dpi)),
        ulx::recpos(0, 0), current_render_scene.get_layout(), pxoff, pyoff,
        current_render_scene.get_padding(), current_render_scene.get_alignment()
    );

    if (push_texture_images_to_gpu)
        push_texture_images_gpu();
}

inline auto ulx::context::set_timer(const ticktimer& timer) -> void { this->tick_timer = timer; }
inline auto ulx::context::dirtied(bool data_dirty) -> void { render_dirty = true; this->data_dirty = data_dirty; }
inline auto ulx::context::titled() const -> bool { return window_titled_state; }
inline auto ulx::context::resizable() const -> bool { return window_resizable_state; }
inline auto ulx::context::dpi() const -> UINT { return window_dpi; }

inline auto ulx::context::window_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT {
    switch (message) {
        case WM_NCCREATE: {
            auto* create_structure = reinterpret_cast<CREATESTRUCTW*>(lparam);
            auto* ctx = static_cast<context*>(create_structure->lpCreateParams);

            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
            return DefWindowProcW(hwnd, message, wparam, lparam);
        } case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } case WM_NCDESTROY: {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
            return DefWindowProcW(hwnd, message, wparam, lparam);
        } case WM_NCHITTEST: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);
            if (!ctx->resizable() || IsZoomed(hwnd)) return HTCLIENT;

            POINT cursor_position; GetCursorPos(&cursor_position);
            RECT window_rect; GetWindowRect(hwnd, &window_rect);
            int frame = GetSystemMetricsForDpi(SM_CYFRAME, ctx->dpi());

            // Top edge
            if (cursor_position.y >= window_rect.top - frame && cursor_position.y < window_rect.top + frame)
                return HTTOP;

            return DefWindowProcW(hwnd, message, wparam, lparam); // Another edges and corner cases
        } case WM_NCCALCSIZE: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx || ctx->titled()) return DefWindowProcW(hwnd, message, wparam, lparam);

            /* Move the top of the client area to the top of the window
                to cover the titlebar */
            auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
            params->rgrc[0].top -= ctx->window_titlebar_height;

            return DefWindowProcW(hwnd, WM_NCCALCSIZE, wparam, lparam);
        } case WM_PAINT: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            ctx->draw_frame();

            ValidateRect(hwnd, nullptr);
            return 0;
        } case WM_GETMINMAXINFO: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            ulx::recsize min_size = ctx->window_min_size;
            ulx::recsize max_size = ctx->window_max_size;
            min_size = ulx::recsize(ulx::wfn::logical_cast(min_size.width()), ulx::wfn::logical_cast(min_size.height()));
            max_size = ulx::recsize(ulx::wfn::logical_cast(max_size.width()), ulx::wfn::logical_cast(max_size.height()));

            MINMAXINFO* minmax_info = reinterpret_cast<MINMAXINFO*>(lparam);
            minmax_info->ptMinTrackSize.x = min_size.width();
            minmax_info->ptMinTrackSize.y = min_size.height();
            minmax_info->ptMaxTrackSize.x = max_size.width();
            minmax_info->ptMaxTrackSize.y = max_size.height();

            return 0;
        } case WM_SIZE: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            if (ctx->first_frame) {
                ctx->first_frame = false;
                return DefWindowProcW(hwnd, message, wparam, lparam);
            }

            RECT rect; GetClientRect(hwnd, &rect);
            ctx->window_current_height = rect.bottom - rect.top;
            ctx->window_current_width = rect.right - rect.left;
            ctx->update_push_constant();

            ctx->dirtied(true);
            ctx->draw_frame();

            return 0;
        } case WM_SIZING: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            RECT rect; GetClientRect(hwnd, &rect);
            ctx->window_current_height = rect.bottom - rect.top;
            ctx->window_current_width = rect.right - rect.left;
            ctx->update_push_constant();

            ctx->dirtied(true);
            ctx->draw_frame();

            return 0;
        } case WM_EXITSIZEMOVE: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            RECT rc;
            for (ulx::u8 i = 0; i < 2; i++) {
                GetClientRect(hwnd, &rc);
                SendMessageW(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
                GetWindowRect(hwnd, &rc);
                SendMessageW(hwnd, WM_SIZING, WMSZ_BOTTOMRIGHT, reinterpret_cast<LPARAM>(&rc));
            }

            return 0;
        }
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

inline auto ulx::context::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] auto* user_data) -> VkBool32 {
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            ulx::log::warn(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            ulx::log::err(callback_data->pMessage);
        default: break;
    }

    return VK_FALSE;
}

inline auto ulx::context::create_window(const ulx::wininfo& window_info) -> void {
    ulx::str window_class_name = window_info.get_window_class_name();
    ulx::str window_title = window_info.get_window_title();
    ulx::wstr window_wide_class_name = ulx::wstr(window_class_name.begin(), window_class_name.end());
    ulx::wstr window_wide_title = ulx::wstr(window_title.begin(), window_title.end());
    
    ulx::recsize window_size = window_info.get_initial_window_size();
    ulx::recpos window_pos = window_info.get_initial_window_pos();
    
    window_min_size = window_info.get_min_window_size();
    window_max_size = window_info.get_max_window_size();
    if (window_size.width() > window_min_size.width()) window_size = window_min_size;
    if (window_size.height() > window_min_size.height()) window_size = window_min_size;
    if (window_size.width() > window_max_size.width()) window_size = window_max_size;
    if (window_size.height() > window_max_size.height()) window_size = window_max_size;
    ulx::u32 physical_window_width = ulx::wfn::physical_cast(window_size.width());
    ulx::u32 physical_window_height = ulx::wfn::physical_cast(window_size.height());
    ulx::i32 physical_window_x = ulx::wfn::physical_cast(window_pos.x());
    ulx::i32 physical_window_y = ulx::wfn::physical_cast(window_pos.y());

    DWORD window_style = WS_OVERLAPPED |  WS_CAPTION |  WS_THICKFRAME |  WS_MINIMIZEBOX |  WS_MAXIMIZEBOX;
    if (window_info.get_window_attrs() & ulx::winattr::titled) {
        window_titled_state = true;
    } else if (!(window_info.get_window_attrs() & ulx::winattr::bordered)) {
        window_style &= ~WS_BORDER;
        window_resizable_state = false;
    }

    if (window_info.get_window_attrs() & ulx::winattr::resizable) {
        window_resizable_state = true;
    } else {
        window_style &= ~WS_THICKFRAME;
        window_style &= ~WS_MAXIMIZEBOX;
    }

    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = window_process;
    wcex.hInstance = ulx::wfn::hinstance;
    wcex.hCursor = LoadCursorW(nullptr, ulx::wfn::make_int_resource(32512));
    wcex.lpszClassName = window_wide_class_name.c_str();
    ulx::log::expect(RegisterClassExW(&wcex), "context.hpp: create_window(): RegisterClassExW(): failed to register window class, error code: {}", GetLastError());

    window_hwnd = CreateWindowExW(
        0, window_wide_class_name.c_str(), window_wide_title.c_str(),
        window_style, physical_window_x, physical_window_y, physical_window_width, physical_window_height,
        nullptr, nullptr, ulx::wfn::hinstance, this);
    ulx::log::expect(window_hwnd, "context.hpp: create_window(): CreateWindowExW(): failed to create window, error code: {}", GetLastError());
    

    window_dpi = GetDpiForWindow(window_hwnd);
    SetWindowLongPtrW(window_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Update window
    int caption_height = GetSystemMetricsForDpi(SM_CYCAPTION, window_dpi);
    int border_padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, window_dpi);
    int border_size = GetSystemMetricsForDpi(SM_CXBORDER, window_dpi);
    window_titlebar_height = caption_height + border_padding + border_size*2 + 1;
    SetWindowPos(window_hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    RECT rect; GetClientRect(window_hwnd, &rect);
    window_current_width = rect.right - rect.left;
    window_current_height = rect.bottom - rect.top;
}

template<typename AI, typename WI, typename RI>
    requires ulx::expect<AI, ulx::appinfo> &&
             ulx::expect<WI, ulx::wininfo> &&
             ulx::expect<RI, ulx::renderinfo>
inline constexpr ulx::context::context(AI&& application_info, WI&& window_info, RI&& render_info) {
    create_window(std::forward<WI>(window_info));
    create_vulkan_objects(std::forward<AI>(application_info), std::forward<RI>(render_info));
}

inline ulx::context::~context() { destroy_vulkan_objects(); }

inline auto ulx::context::exec() -> int {
    MSG msg{}; while (true) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return msg.wParam;
            DispatchMessageW(&msg);
        } else if (render_dirty) draw_frame();
        else WaitMessage();
    }

    return 0;
}

inline auto ulx::context::set_min(const ulx::recsize& size) -> void { window_min_size = size; }
inline auto ulx::context::set_max(const ulx::recsize& size) -> void { window_max_size = size; }
inline auto ulx::context::show() const -> void { ShowWindow(window_hwnd, SW_SHOW); UpdateWindow(window_hwnd); }
inline auto ulx::context::hide() const -> void { ShowWindow(window_hwnd, SW_HIDE); UpdateWindow(window_hwnd); }
inline auto ulx::context::exit() const -> void { PostMessageW(window_hwnd, WM_CLOSE, 0, 0); }
inline auto ulx::context::size() const -> ulx::recsize { return ulx::recsize(ulx::wfn::logical_cast(window_current_width), ulx::wfn::logical_cast(window_current_height)); }
inline auto ulx::context::width() const -> ulx::f32 { return ulx::wfn::logical_cast(window_current_width); }
inline auto ulx::context::height() const -> ulx::f32 { return ulx::wfn::logical_cast(window_current_height); }
inline auto ulx::context::pos() const -> ulx::recpos { 
    RECT window_rect{}; GetWindowRect(window_hwnd, &window_rect);
    return ulx::recpos(ulx::wfn::logical_cast(window_rect.left), ulx::wfn::logical_cast(window_rect.top));
}
inline auto ulx::context::x() const -> ulx::f32 { return pos().x(); }
inline auto ulx::context::y() const -> ulx::f32 { return pos().y(); }


inline auto ulx::context::set_pos(const ulx::recpos& pos) -> void {
    ulx::i32 physical_x = ulx::wfn::physical_cast(pos.x());
    ulx::i32 physical_y = ulx::wfn::physical_cast(pos.y());
    MoveWindow(window_hwnd, physical_x, physical_y, window_current_width, window_current_height, TRUE);
}

inline auto ulx::context::set_size(const ulx::recsize& size) -> void {
    ulx::u32 physical_width = ulx::wfn::physical_cast(size.width());
    ulx::u32 physical_height = ulx::wfn::physical_cast(size.height());
    MoveWindow(window_hwnd, x(), y(), physical_width, physical_height, TRUE);
    window_current_width = physical_width;
    window_current_height = physical_height;
}

inline auto ulx::context::create_vulkan_instance() -> void {
    VkApplicationInfo vulkan_application_info{};
    vulkan_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vulkan_application_info.pApplicationName = application_id.c_str();
    vulkan_application_info.applicationVersion = application_version;
    vulkan_application_info.pEngineName = "No Engine";
    vulkan_application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vulkan_application_info.apiVersion = VK_API_VERSION_1_2;
    VkInstanceCreateInfo vulkan_instance_create_info{};
    vulkan_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vulkan_instance_create_info.pApplicationInfo = &vulkan_application_info;
    vulkan_instance_create_info.enabledLayerCount = 0;
    vulkan_instance_create_info.ppEnabledLayerNames = __uii::vkreqs::required_validation_layer;
    vulkan_instance_create_info.enabledExtensionCount = 2;
    vulkan_instance_create_info.ppEnabledExtensionNames = __uii::vkreqs::required_extensions;
    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info;

    #ifndef ULIXRELEASE
        ulx::log::expect(check_validation_layer_support(), "context.hpp: create_vulkan_instance(): check_validation_layer_support(): requested validation layers not available");

        vulkan_instance_create_info.enabledLayerCount++;
        vulkan_instance_create_info.enabledExtensionCount++;

        debug_utils_messenger_create_info = {};
        debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_utils_messenger_create_info.pfnUserCallback = debug_callback;
        vulkan_instance_create_info.pNext = &debug_utils_messenger_create_info;
    #endif

    ulx::log::vkexp(vkCreateInstance(&vulkan_instance_create_info, VK_NULL_HANDLE, &vulkan_instance),
        "context.hpp: create_vulkan_instance(): vkCreateInstance(): failed to create vulkan instance");
}

inline auto ulx::context::check_validation_layer_support() -> bool {
    ulx::u32 layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, VK_NULL_HANDLE);
    ulx::vec<VkLayerProperties> available_layers = ulx::vec<VkLayerProperties>(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const auto& layer_properties : available_layers) {
        if (std::strcmp("VK_LAYER_KHRONOS_validation", layer_properties.layerName) == 0)
            return true;
    }
    return false;
}

inline auto ulx::context::create_debug_messenger() -> void {
    #ifndef ULIXRELEASE
        VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info;
        populate_debug_messenger_create_info(debug_utils_messenger_create_info);
    
        auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCreateDebugUtilsMessengerEXT"));
        ulx::log::expect(vkCreateDebugUtilsMessengerEXT, "context.hpp: create_debug_messenger(): vkGetInstanceProcAddr(): failed to get function pointer vkCreateDebugUtilsMessengerEXT from Vulkan instance");
        ulx::log::vkexp(vkCreateDebugUtilsMessengerEXT(vulkan_instance, &debug_utils_messenger_create_info, VK_NULL_HANDLE, &debug_messenger),
            "context.hpp: create_debug_messenger(): vkCreateDebugUtilsMessengerEXT(): failed to create debug messenger");
    #endif
}

inline auto ulx::context::destroy_debug_utils_messenger() -> void {
    if (debug_messenger == VK_NULL_HANDLE)
        return;

    auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vulkan_instance, "vkDestroyDebugUtilsMessengerEXT");
    ulx::log::expect(vkDestroyDebugUtilsMessengerEXT, "context.hpp: destroy_debug_utils_messenger(): vkGetInstanceProcAddr(): failed to get function pointer vkDestroyDebugUtilsMessengerEXT from Vulkan instance");
    vkDestroyDebugUtilsMessengerEXT(vulkan_instance, debug_messenger, VK_NULL_HANDLE);
}

inline auto ulx::context::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) -> void {
    debug_utils_messenger_create_info = {};
    debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_utils_messenger_create_info.pfnUserCallback = debug_callback;
}

inline auto ulx::context::select_physical_device() -> void {
    ulx::u32 physical_device_count = 0;
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, VK_NULL_HANDLE);
    ulx::log::expect(physical_device_count > 0, "context.hpp: select_physical_device(): vkEnumeratePhysicalDevices(): failed to find GPUs with Vulkan support");
    
    ulx::vec<VkPhysicalDevice> physical_devices = ulx::vec<VkPhysicalDevice>(physical_device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, physical_devices.data());

    VkPhysicalDevice alternative_device = VK_NULL_HANDLE;
    __uii::vkclses::PhysicalDeviceInfos alternative_device_infos;
    ulx::u32 last_discrete_gpu_score = 0;
    ulx::u32 last_alternative_device_score = 0;

    for (auto& device : physical_devices) {
        __uii::vkclses::PhysicalDeviceInfos device_infos = __uii::vkclses::PhysicalDeviceInfos(device, window_surface);
        if (!device_infos.is_suitable()) continue;
        ulx::u32 score = __uii::vkalg::get_device_score(device_infos);
        if (device_infos.match_gpu_type(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && score > last_discrete_gpu_score) {
            last_discrete_gpu_score = score;
            physical_device = device;
            physical_device_infos = device_infos;
        } else if (device_infos.match_gpu_type(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && score > last_alternative_device_score) {
            last_alternative_device_score = score;
            alternative_device = device;
            alternative_device_infos = device_infos;
        }
    }

    if (physical_device != VK_NULL_HANDLE) return;
    ulx::log::expect(alternative_device != VK_NULL_HANDLE, "context.hpp: select_physical_device(): failed to select a suitable GPU");
    
    physical_device = alternative_device;
    physical_device_infos = alternative_device_infos;
}

inline auto ulx::context::create_logical_device() -> void {
    ulx::vec<VkDeviceQueueCreateInfo> device_queue_create_infos;
    ulx::set<ulx::u32> unique_queue_family_indices = physical_device_infos.get_unique_queue_family_indices();
    float queue_priorities = 1.0f;
    for (ulx::u32 index : unique_queue_family_indices) {
        VkDeviceQueueCreateInfo device_queue_create_info{};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.queueFamilyIndex = index;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.pQueuePriorities = &queue_priorities;
        device_queue_create_infos.push_back(device_queue_create_info);
    }

    VkPhysicalDeviceFeatures physical_device_features{};
    physical_device_features.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = static_cast<ulx::u32>(device_queue_create_infos.size());
    device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
    device_create_info.pEnabledFeatures = &physical_device_features;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = __uii::vkreqs::enabled_extension;
    device_create_info.pNext = &features12;
    ulx::log::vkexp(vkCreateDevice(physical_device, &device_create_info, VK_NULL_HANDLE, &logical_device),
        "context.hpp: create_logical_device(): vkCreateDevice(): failed to create logical device");
    

    device_queues = __uii::vkclses::DeviceQueues(logical_device, physical_device_infos);
}

inline auto ulx::context::create_window_surface() -> void {
    VkWin32SurfaceCreateInfoKHR win32_surface_create_info{};
    win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32_surface_create_info.hwnd = window_hwnd;
    win32_surface_create_info.hinstance = ulx::wfn::hinstance;

    ulx::log::vkexp(vkCreateWin32SurfaceKHR(vulkan_instance, &win32_surface_create_info, VK_NULL_HANDLE, &window_surface),
        "context.hpp: create_window_surface(): vkCreateWin32SurfaceKHR(): failed to create win32 window surface");
}

inline auto ulx::context::create_swapchain() -> void {
    __uii::vkclses::SwapchainSupportDetails& swapchain_support_details = physical_device_infos.swapchain_support_details;
    __uii::vkclses::OptionalQueueFamilyIndices& optional_queue_family_indices = physical_device_infos.queue_family_indices;
    VkSurfaceFormatKHR surface_format = swapchain_support_details.select_surface_format();
    VkExtent2D extent = swapchain_support_details.select_extent(window_hwnd);
    ulx::u32 queue_family_indices[] = { optional_queue_family_indices.graphics_queue_family_index.value(), optional_queue_family_indices.present_queue_family_index.value() };
    ulx::u32 min_image_count = swapchain_support_details.surface_capabilities.minImageCount + 1;
    if (swapchain_support_details.surface_capabilities.maxImageCount > 0 && min_image_count > swapchain_support_details.surface_capabilities.maxImageCount)
        min_image_count = swapchain_support_details.surface_capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = window_surface;
    swapchain_create_info.minImageCount = min_image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = swapchain_support_details.surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (optional_queue_family_indices.graphics_queue_family_index != optional_queue_family_indices.present_queue_family_index) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices; };
    ulx::log::vkexp(vkCreateSwapchainKHR(logical_device, &swapchain_create_info, VK_NULL_HANDLE, &swapchain),
        "context.hpp: create_swapchain(): vkCreateSwapchainKHR(): failed to create swapchain");
    

    swapchain_image_format = surface_format.format;
    swapchain_extent = extent;

    get_swapchain_images();
}

inline auto ulx::context::get_swapchain_images() -> void {
    ulx::u32 image_count = 0;
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, VK_NULL_HANDLE);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, swapchain_images.data());

    max_frames_in_flight = static_cast<ulx::size>(image_count);
}

inline auto ulx::context::create_swapchain_image_views() -> void {
    swapchain_image_views.resize(swapchain_images.size());

    for (ulx::size index = 0; index < swapchain_images.size(); index++)
        swapchain_image_views[index] = __uii::vkalg::create_image_view(logical_device, swapchain_images[index], swapchain_image_format);
}

inline auto ulx::context::create_shader_module(unsigned char* code, unsigned int length) -> VkShaderModule {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = length;
    shader_module_create_info.pCode = reinterpret_cast<ulx::u32*>(code);

    VkShaderModule shader_module;
    ulx::log::vkexp(vkCreateShaderModule(logical_device, &shader_module_create_info, VK_NULL_HANDLE, &shader_module),
        "context.hpp: create_shader_module(): vkCreateShaderModule(): failed to create shader module");
    

    return shader_module;
}

inline auto ulx::context::populate_pipeline_shader_stage_create_info(VkPipelineShaderStageCreateInfo& create_info, VkShaderModule shader_module, VkShaderStageFlagBits stage) -> void {
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage = stage;
    create_info.module = shader_module;
    create_info.pName = "main";
}

inline auto ulx::context::create_graphics_pipeline() -> void {
    VkShaderModule fragment_shader_module = create_shader_module(fragment_shader, fragment_shader_len);
    VkShaderModule vertex_shader_module = create_shader_module(vertex_shader, vertex_shader_len);

    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info{};
    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
    populate_pipeline_shader_stage_create_info(fragment_shader_stage_create_info, fragment_shader_module, VK_SHADER_STAGE_FRAGMENT_BIT);
    populate_pipeline_shader_stage_create_info(vertex_shader_stage_create_info, vertex_shader_module, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_infos[] = { fragment_shader_stage_create_info, vertex_shader_stage_create_info };
    VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
    pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipeline_dynamic_state_create_info.dynamicStateCount = 2;
    pipeline_dynamic_state_create_info.pDynamicStates = __uii::vkreqs::dynamic_states;

    VkVertexInputBindingDescription vertex_input_binding_description = __uii::vsdces::vertex2d::get_binding_description();
    ulx::vec<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions = __uii::vsdces::vertex2d::get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};
    pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
    pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
    pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<ulx::u32>(vertex_input_attribute_descriptions.size());
    pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
    pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain_extent.width);
    viewport.height = static_cast<float>(swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_extent;

    VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info{};
    pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipeline_viewport_state_create_info.viewportCount = 1;
    pipeline_viewport_state_create_info.pViewports = &viewport;
    pipeline_viewport_state_create_info.scissorCount = 1;
    pipeline_viewport_state_create_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};
    pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    pipeline_rasterization_state_create_info.lineWidth = 1.0f;
    pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
    pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo pipeline_mulisample_state_create_info{};
    pipeline_mulisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipeline_mulisample_state_create_info.sampleShadingEnable = VK_FALSE;
    pipeline_mulisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_mulisample_state_create_info.minSampleShading = 1.0f;
    pipeline_mulisample_state_create_info.pSampleMask = VK_NULL_HANDLE;
    pipeline_mulisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    pipeline_mulisample_state_create_info.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state{};
    pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_color_blend_attachment_state.blendEnable = VK_TRUE;
    pipeline_color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    pipeline_color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    pipeline_color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipeline_color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipeline_color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};
    pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipeline_color_blend_state_create_info.logicOpEnable = VK_FALSE;
    pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    pipeline_color_blend_state_create_info.attachmentCount = 1;
    pipeline_color_blend_state_create_info.pAttachments = &pipeline_color_blend_attachment_state;
    pipeline_color_blend_state_create_info.blendConstants[0] = 0.0f;
    pipeline_color_blend_state_create_info.blendConstants[1] = 0.0f;
    pipeline_color_blend_state_create_info.blendConstants[2] = 0.0f;
    pipeline_color_blend_state_create_info.blendConstants[3] = 0.0f;

    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(__uii::vsdces::PushConstant);

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;
    ulx::log::vkexp(vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, VK_NULL_HANDLE, &pipeline_layout),
        "context.hpp: create_graphics_pipeline(): vkCreatePipelineLayout(): failed to create pipeline layout");


    __uii::vkalg::ensure_cache_exists(cache_file_path);
    ulx::vec<char> cache_data = __uii::vkalg::read_cache(cache_file_path);
    VkPipelineCacheCreateInfo pipeline_cache_create_info{};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipeline_cache_create_info.initialDataSize = cache_data.size();
    pipeline_cache_create_info.pInitialData = cache_data.data();
    VkPipelineCache pipeline_cache;
    ulx::log::vkexp(vkCreatePipelineCache(logical_device, &pipeline_cache_create_info, nullptr, &pipeline_cache),
        "context.hpp: create_graphics_pipeline(): vkCreatePipelineCache(): failed to create pipeline cache");
    

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.stageCount = 2;
    graphics_pipeline_create_info.pStages = pipeline_shader_stage_create_infos;
    graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
    graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &pipeline_mulisample_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState = VK_NULL_HANDLE;
    graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
    graphics_pipeline_create_info.layout = pipeline_layout;
    graphics_pipeline_create_info.renderPass = render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;
    ulx::log::vkexp(vkCreateGraphicsPipelines(logical_device, pipeline_cache, 1, &graphics_pipeline_create_info, VK_NULL_HANDLE, &graphics_pipeline),
        "context.hpp: create_graphics_pipeline(): vkCreateGraphicsPipelines(): failed to create graphics pipeline");


    size_t cache_data_size;
    ulx::vec<char> new_cache_data;
    vkGetPipelineCacheData(logical_device, pipeline_cache, &cache_data_size, nullptr);
    new_cache_data.resize(cache_data_size);
    vkGetPipelineCacheData(logical_device, pipeline_cache, &cache_data_size, new_cache_data.data());
    __uii::vkalg::write_cache(cache_file_path, new_cache_data);
    vkDestroyPipelineCache(logical_device, pipeline_cache, nullptr);
    vkDestroyShaderModule(logical_device, fragment_shader_module, VK_NULL_HANDLE);
    vkDestroyShaderModule(logical_device, vertex_shader_module, VK_NULL_HANDLE);
}

inline auto ulx::context::create_render_pass() -> void {
    VkAttachmentDescription color_attachment_description{};
    color_attachment_description.format = swapchain_image_format;
    color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_reference{};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment_reference;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment_description;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;
    ulx::log::vkexp(vkCreateRenderPass(logical_device, &render_pass_create_info, VK_NULL_HANDLE, &render_pass),
        "context.hpp: create_render_pass(): vkCreateRenderPass(): failed to create render pass");
}

inline auto ulx::context::create_swapchain_frame_buffers() -> void {
    swapchain_frame_buffers.resize(swapchain_images.size());
    
    std::for_each(std::execution::par, std::views::iota(size_t{0}, swapchain_images.size()).begin(),
                  std::views::iota(size_t{0}, swapchain_images.size()).end(), [&](ulx::size index) {
        VkFramebufferCreateInfo frame_buffer_create_info{};
        frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_create_info.renderPass = render_pass;
        frame_buffer_create_info.attachmentCount = 1;
        frame_buffer_create_info.pAttachments = &swapchain_image_views[index];
        frame_buffer_create_info.width = swapchain_extent.width;
        frame_buffer_create_info.height = swapchain_extent.height;
        frame_buffer_create_info.layers = 1;
        
        ulx::log::vkexp(vkCreateFramebuffer(logical_device, &frame_buffer_create_info, VK_NULL_HANDLE, &swapchain_frame_buffers[index]),
            "context.hpp: create_swapchain_frame_buffers(): vkCreateFramebuffer(): failed to create frame buffers");
    });
}

inline auto ulx::context::create_command_pool() -> void {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = physical_device_infos.queue_family_indices.graphics_queue_family_index.value();
    ulx::log::vkexp(vkCreateCommandPool(logical_device, &command_pool_create_info, VK_NULL_HANDLE, &command_pool),
        "context.hpp: create_command_pool(): vkCreateCommandPool(): failed to create command pool");
}

inline auto ulx::context::create_command_buffers() -> void {
    command_buffers.resize(max_frames_in_flight);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = static_cast<ulx::u32>(command_buffers.size());
    ulx::log::vkexp(vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, command_buffers.data()),
        "context.hpp: create_command_buffers(): vkAllocateCommandBuffers(): failed to allocate command buffer");
}

inline auto ulx::context::create_sync_objects() -> void {
    image_available_semaphores.resize(max_frames_in_flight);
    render_finished_semaphores.resize(max_frames_in_flight);
    in_flight_fences.resize(max_frames_in_flight);

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (ulx::size index = 0; index < max_frames_in_flight; index++) {
        if (vkCreateSemaphore(logical_device, &semaphore_create_info, VK_NULL_HANDLE, &image_available_semaphores[index]) != VK_SUCCESS)
            ulx::log::err("failed to create sync object: image available semaphore");
        if (vkCreateSemaphore(logical_device, &semaphore_create_info, VK_NULL_HANDLE, &render_finished_semaphores[index]) != VK_SUCCESS)
            ulx::log::err("failed to create sync object: render finished semaphore");
        if (vkCreateFence(logical_device, &fence_create_info, VK_NULL_HANDLE, &in_flight_fences[index]) != VK_SUCCESS)
            ulx::log::err("failed to create sync object: in flight fence");
    }
}

inline auto ulx::context::destroy_swapchain() -> void {
    for (const auto& frame_buffer : swapchain_frame_buffers)
        vkDestroyFramebuffer(logical_device, frame_buffer, VK_NULL_HANDLE);
    for (const auto& image_view : swapchain_image_views)
        vkDestroyImageView(logical_device, image_view, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(logical_device, swapchain, VK_NULL_HANDLE);
}

inline auto ulx::context::recreate_swapchain() -> void {
    RECT window_rect;
    GetWindowRect(window_hwnd, &window_rect);
    ulx::u32 width = window_rect.right - window_rect.left;
    ulx::u32 height = window_rect.bottom - window_rect.top;
    if (width == 0 || height == 0) return;

    vkDeviceWaitIdle(logical_device);
    destroy_swapchain();

    create_swapchain();
    create_swapchain_image_views();
    create_swapchain_frame_buffers();
}

inline auto ulx::context::create_staging_buffer() -> void {
    VkDeviceSize texture_size = create_texture_images();
    VkDeviceSize vertex_size = render_vertices.size() * sizeof(__uii::vsdces::vertex2d);
    VkDeviceSize index_size = render_indices.size() * sizeof(ulx::u32);

    staging_buffer_size = std::max(std::max(vertex_size, index_size), texture_size);
    staging_buffer = __uii::vkclses::Buffer(
        physical_device, logical_device, staging_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
}

inline auto ulx::context::create_vertex_buffer() -> void {
    VkDeviceSize size = render_vertices.size() * sizeof(__uii::vsdces::vertex2d);
    vertex_buffer = __uii::vkclses::Buffer(
        physical_device, logical_device, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
}

inline auto ulx::context::create_index_buffer() -> void {
    VkDeviceSize size = render_indices.size() * sizeof(ulx::u32);
    index_buffer = __uii::vkclses::Buffer(
        physical_device, logical_device, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
}

inline auto ulx::context::update_object_buffers() -> void {
    staging_buffer.map_memory(logical_device, render_vertices.data(), render_vertices.size() * sizeof(__uii::vsdces::vertex2d));
    staging_buffer.copy_buffer_to(vertex_buffer, logical_device, command_pool, device_queues.graphics_queue, render_vertices.size() * sizeof(__uii::vsdces::vertex2d));
    staging_buffer.map_memory(logical_device, render_indices.data(), render_indices.size() * sizeof(ulx::u32));
    staging_buffer.copy_buffer_to(index_buffer, logical_device, command_pool, device_queues.graphics_queue, render_indices.size() * sizeof(ulx::u32));
}

inline auto ulx::context::record_command_buffer(VkCommandBuffer command_buffer, ulx::u32 image_index) -> void {
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = VK_NULL_HANDLE;
    ulx::log::vkexp(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info),
        "context.hpp: record_command_buffer(): vkBeginCommandBuffer(): failed to begin command buffer");

    VkClearValue clear_color = {{{current_background_color.red() / 255.0f, current_background_color.green() / 255.0f, current_background_color.blue() / 255.0f, current_background_color.alpha() / 255.0f}}};
    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = swapchain_frame_buffers[image_index];
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = swapchain_extent;
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;
    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain_extent.width);
    viewport.height = static_cast<float>(swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    constexpr VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer.buffer, offsets);
    vkCmdBindIndexBuffer(command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(__uii::vsdces::PushConstant), &push_constant);
    vkCmdDrawIndexed(command_buffer, static_cast<ulx::u32>(render_indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    ulx::log::vkexp(vkEndCommandBuffer(command_buffer), 
        "context.hpp: record_command_buffer(): vkEndCommandBuffer(): failed to end command buffer");
}

inline auto ulx::context::draw_frame() -> void {
    vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
    render_dirty = false;

    if (data_dirty) {
        data_dirty = false;
        update_scene_objects();
        update_object_buffers();
    }

    ulx::u32 image_index;
    VkResult acquire_result = vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return;
    } else ulx::log::expect(acquire_result == VK_SUCCESS || acquire_result == VK_SUBOPTIMAL_KHR, 
        "failed to acquire next image");

    vkResetFences(logical_device, 1, &in_flight_fences[current_frame]);
    vkResetCommandBuffer(command_buffers[current_frame], 0);
    record_command_buffer(command_buffers[current_frame], image_index);

    constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_finished_semaphores[current_frame];
    ulx::log::vkexp(vkQueueSubmit(device_queues.graphics_queue, 1, &submit_info, in_flight_fences[current_frame]), 
        "context.hpp: draw_frame(): vkQueueSubmit(): failed to submit queue");
    

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_finished_semaphores[current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.pImageIndices = &image_index;
    vkQueuePresentKHR(device_queues.present_queue, &present_info);

    current_frame = (current_frame + 1) % max_frames_in_flight;
    tick_timer.update(*this);
}

inline auto ulx::context::create_descriptor_set_layout() -> void {
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
    descriptor_set_layout_binding.binding = 0;
    descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_set_layout_binding.descriptorCount = max_texture_count;
    descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptor_set_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
    ulx::log::vkexp(vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout), 
        "context.hpp: create_descriptor_set_layout(): vkCreateDescriptorSetLayout(): failed to create descriptor set layout");
}

inline auto ulx::context::create_descriptor_update_template() -> void {
    VkDescriptorUpdateTemplateEntry descriptor_update_template_entry{};
    descriptor_update_template_entry.dstBinding = 0;
    descriptor_update_template_entry.dstArrayElement = 0;
    descriptor_update_template_entry.descriptorCount = max_texture_count;
    descriptor_update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_update_template_entry.offset = 0;
    descriptor_update_template_entry.stride = sizeof(VkDescriptorImageInfo);

    VkDescriptorUpdateTemplateCreateInfo descriptor_update_template_create_info{};
    descriptor_update_template_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
    descriptor_update_template_create_info.descriptorUpdateEntryCount = 1;
    descriptor_update_template_create_info.pDescriptorUpdateEntries = &descriptor_update_template_entry;
    descriptor_update_template_create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    descriptor_update_template_create_info.descriptorSetLayout = descriptor_set_layout;
    ulx::log::vkexp(vkCreateDescriptorUpdateTemplate(logical_device, &descriptor_update_template_create_info, nullptr, &descriptor_update_template),
        "context.hpp: create_descriptor_update_template(): vkCreateDescriptorUpdateTemplate(): failed to create descriptor update template");
}

inline auto ulx::context::create_descriptor_pool() -> void {
    VkDescriptorPoolSize descriptor_pool_size{};
    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_pool_size.descriptorCount = max_texture_count;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;
    descriptor_pool_create_info.maxSets = 1;
    ulx::log::vkexp(vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &descriptor_pool),
        "context.hpp: create_descriptor_pool(): vkCreateDescriptorPool(): failed to create descriptor pool");
    
}

inline auto ulx::context::create_descriptor_set() -> void {
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set_layout;

    ulx::log::vkexp(vkAllocateDescriptorSets(logical_device, &alloc_info, &descriptor_set),
        "context.hpp: create_descriptor_set(): vkAllocateDescriptorSets(): failed to allocate descriptor sets");
    

    update_texture_descriptors();
}

inline auto ulx::context::update_texture_descriptors() -> void {
    ulx::vec<VkDescriptorImageInfo> image_infos(max_texture_count);

    for (ulx::size index = 0; index < max_texture_count; index++) {
        image_infos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_infos[index].imageView = texture_images[0].image_view;
        image_infos[index].sampler = texture_image_sampler;
    }

    for (ulx::size index = 0; index < texture_images.size(); index++)
        image_infos[index].imageView = texture_images[index].image_view;
    vkUpdateDescriptorSetWithTemplate(logical_device, descriptor_set, descriptor_update_template, image_infos.data());
}

inline auto ulx::context::map_texture_images() -> void {
    ulx::vec<VkDeviceSize> offsets(texture_image_pixmaps.size());
    VkDeviceSize current_offset = 0;
    ulx::u32 memory_type_bits;

    for (ulx::size index = 0; index < texture_image_pixmaps.size(); ++index) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(logical_device, texture_images[index].image, &memory_requirements);
        current_offset = (current_offset + memory_requirements.alignment - 1) & ~(memory_requirements.alignment - 1);
        offsets[index] = current_offset;
        current_offset += memory_requirements.size;

        if (!index) memory_type_bits = memory_requirements.memoryTypeBits;
        else memory_type_bits &= memory_requirements.memoryTypeBits;
    }

    VkDeviceSize staging_total_size = 0;
    for (auto& pixmap : texture_image_pixmaps) 
        staging_total_size += pixmap.get_pixel_size();

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = current_offset;
    memory_allocate_info.memoryTypeIndex = __uii::vkalg::find_memory_type(physical_device, memory_type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ulx::log::vkexp(vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, &texture_image_memory),
        "context.hpp: map_texture_images(): vkAllocateMemory(): failed to allocate texture image memory");
    


    for (ulx::size index = 0; index < texture_image_pixmaps.size(); index++)
        vkBindImageMemory(logical_device, texture_images[index].image, texture_image_memory, offsets[index]);

    void* mapped; VkDeviceSize staging_offset = 0;
    vkMapMemory(logical_device, staging_buffer.buffer_memory, 0, staging_total_size, 0, &mapped);
    VkCommandBuffer single_time_command_buffer = __uii::vkalg::start_single_time_command_buffer(logical_device, command_pool);
    for (ulx::size index = 0; index < texture_image_pixmaps.size(); index++) {
        VkImage image = texture_images[index].image;
        ulx::pixmap pixmap = texture_image_pixmaps[index];
        ulx::recsize size = pixmap.get_size();

        memcpy((char*)mapped + staging_offset, pixmap.get_pixels().data(), pixmap.get_pixel_size());
        offsets[index] = staging_offset;
        staging_offset += pixmap.get_pixel_size();

        __uii::vkalg::transition_image_layout(logical_device, single_time_command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        __uii::vkalg::copy_buffer_to_image(logical_device, staging_buffer.buffer, single_time_command_buffer, image, size.width(), size.height(), offsets[index]);
        __uii::vkalg::transition_image_layout(logical_device, single_time_command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    __uii::vkalg::end_single_time_command_buffer(logical_device, device_queues.graphics_queue, command_pool, single_time_command_buffer);
    vkUnmapMemory(logical_device, staging_buffer.buffer_memory);
}

inline auto ulx::context::create_texture_image(ulx::size index) -> VkMemoryRequirements {
    VkImage& texture = texture_images[index].image;
    ulx::pixmap pixmap = texture_image_pixmaps[index];
    ulx::recsize size = pixmap.get_size();

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = size.width();
    image_create_info.extent.height = size.height();
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ulx::log::vkexp(vkCreateImage(logical_device, &image_create_info, VK_NULL_HANDLE, &texture),
        "context.hpp: create_texture_image(): vkCreateImage(): failed to create texture image");
    

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(logical_device, texture, &memory_requirements);
    return memory_requirements;
}

inline auto ulx::context::create_texture_image_views() -> void {
    for (ulx::size index = 0; index < texture_images.size(); ++index)
        texture_images[index].image_view = __uii::vkalg::create_image_view(logical_device, texture_images[index].image, VK_FORMAT_R8G8B8A8_UNORM);
}

inline auto ulx::context::create_texture_image_sampler() -> void {
    VkSamplerCreateInfo sampler_create_info{};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = physical_device_infos.properties.limits.maxSamplerAnisotropy;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 0.0f;
    ulx::log::vkexp(vkCreateSampler(logical_device, &sampler_create_info, nullptr, &texture_image_sampler),
        "context.hpp: create_texture_image_sampler(): vkCreateSampler(): failed to create texture image sampler");
}

inline auto ulx::context::push_texture_image_cpu(const ulx::pixmap& pixmap) -> void {
    texture_image_pixmaps.push_back(pixmap);
    push_texture_images_to_gpu = true;
}

inline auto ulx::context::push_texture_images_gpu() -> void {
    push_texture_images_to_gpu = false;
    vkDeviceWaitIdle(logical_device);

    for (auto& image : texture_images)
        image.release(logical_device);
    recreate_staging_buffer();

    vkFreeMemory(logical_device, texture_image_memory, VK_NULL_HANDLE);
    map_texture_images();
    create_texture_image_views();

    vkDestroyDescriptorPool(logical_device, descriptor_pool, VK_NULL_HANDLE);
    create_descriptor_pool();
    create_descriptor_set();
}

inline auto ulx::context::recreate_staging_buffer() -> void {
    VkDeviceSize staging_total_size = 0;
    for (auto& pixmap : texture_image_pixmaps)
        staging_total_size += pixmap.get_pixel_size();

    if (staging_buffer_size >= staging_total_size) {
        create_texture_images();
        return;
    }

    staging_buffer.release(logical_device);
    create_staging_buffer();
}

inline auto ulx::context::create_texture_images() -> VkDeviceSize {
    VkDeviceSize size = 0; texture_images.resize(texture_image_pixmaps.size());
    for (ulx::size index = 0; index < texture_image_pixmaps.size(); index++) {
        VkMemoryRequirements memory_requirements = create_texture_image(index);
        size += memory_requirements.size;
    }

    return size;
}

inline auto ulx::context::destroy_vulkan_objects() -> void {
    vkDeviceWaitIdle(logical_device);

    vkDestroyDescriptorUpdateTemplate(logical_device, descriptor_update_template, nullptr);
    destroy_swapchain();
    vkDestroySampler(logical_device, texture_image_sampler, nullptr);
    for (auto& image : texture_images)
        image.release(logical_device);
    vkFreeMemory(logical_device, texture_image_memory, VK_NULL_HANDLE);
    vkDestroyDescriptorPool(logical_device, descriptor_pool, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, VK_NULL_HANDLE);
    index_buffer.release(logical_device);
    vertex_buffer.release(logical_device);
    staging_buffer.release(logical_device);
    for (ulx::size index = 0; index < max_frames_in_flight; index++) {
        vkDestroySemaphore(logical_device, image_available_semaphores[index], VK_NULL_HANDLE);
        vkDestroySemaphore(logical_device, render_finished_semaphores[index], VK_NULL_HANDLE);
        vkDestroyFence(logical_device, in_flight_fences[index], VK_NULL_HANDLE);
    }
    vkDestroyCommandPool(logical_device, command_pool, VK_NULL_HANDLE);
    vkDestroyPipeline(logical_device, graphics_pipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(logical_device, pipeline_layout, VK_NULL_HANDLE);
    vkDestroyRenderPass(logical_device, render_pass, VK_NULL_HANDLE);
    vkDestroyDevice(logical_device, VK_NULL_HANDLE);
    destroy_debug_utils_messenger();
    vkDestroySurfaceKHR(vulkan_instance, window_surface, VK_NULL_HANDLE);
    vkDestroyInstance(vulkan_instance, VK_NULL_HANDLE);
}

inline auto ulx::context::create_vulkan_objects(const ulx::appinfo& application_info, const ulx::renderinfo& render_info) -> void {
    renderer = render_info.get_render_callback();
    texture_image_pixmaps = { __uii::vkalg::empty_pixmap() };
    cache_file_path = application_info.get_pipeline_cache_file_path().get_file_path();
    max_texture_count = render_info.get_max_texture_count();
    application_id = application_info.get_application_id();
    application_version = application_info.get_application_version();

    create_vulkan_instance();
    create_debug_messenger();
    create_window_surface();
    select_physical_device();
    create_logical_device();
    create_swapchain();
    create_swapchain_image_views();
    create_render_pass();
    create_descriptor_set_layout();
    create_descriptor_update_template();
    create_graphics_pipeline();
    create_swapchain_frame_buffers();
    create_command_pool();
    create_staging_buffer();
    map_texture_images();
    create_texture_image_views();
    create_texture_image_sampler();
    create_descriptor_pool();
    create_descriptor_set();

    update_scene_objects();
    update_push_constant();

    create_vertex_buffer();
    create_index_buffer();
    create_command_buffers();
    create_sync_objects();
    update_object_buffers();
}
