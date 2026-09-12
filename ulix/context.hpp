#pragma once

#include <vulkan/vulkan_core.h>
#include <file.hpp>
#include <pixmap.hpp>
#include <renderinfo.hpp>
#include <timer.hpp>
#include <scene.hpp>
#include <__inside_impl/vulkan_classes.hpp>
#include <__inside_impl/vulkan_shader_data_classes.hpp>
#include <__inside_impl/vulkan_algorithm.hpp>
#include <__inside_impl/vulkan_requirements.hpp>
#include <__class_decl/context.hpp>
#include <macros.hpp>
#include <rect.hpp>
#include <log.hpp>
#include <color.hpp>
#include <object.hpp>
#include <wininfo.hpp>
#include <appinfo.hpp>
#include <winfunc.hpp>
#include <types.hpp>
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

#include <__builtin_shaders/fragment_shader.inl>
#include <__builtin_shaders/vertex_shader.inl>



inline auto UContext::update_push_constant() -> void {
    push_constant.projection = glm::ortho(
        0.0f, static_cast<ulx::f32>(window_current_width),
        static_cast<ulx::f32>(window_current_height), 0.0f,
        -1.0f, 1.0f);
}

inline auto UContext::update_texture_images(ulx::rect& rect, const ulx::object& object, ulx::u32& texture_index) -> void {
    ulx::object::TextureType texture_type = object.get_texture_type();
    ulx::object::PixmapType texture_variant = object.get_texture();
    if (texture_type == ulx::object::TextureType::None) {
        texture_index = 0;
    } else if (texture_type == ulx::object::TextureType::Bitmap) {
        ulx::file image_file = std::get<ulx::file>(texture_variant);
        auto it = bitmap_texture_cache.find(image_file);
        if (it != bitmap_texture_cache.end())
            texture_index = it->second;
        else {
            push_texture_image_cpu(ulx::pixmap::from_bitmap(image_file));
            texture_index = texture_image_pixmaps.size() - 1;
            bitmap_texture_cache[image_file] = texture_index;
        }

        ulx::pixmap pixmap = texture_image_pixmaps[texture_index];
        ulx::rect pixrec = pixmap.get_rect();
        if (rect.get_width() == -1) rect = rect.new_with_size(pixrec.get_width(), rect.get_height());
        if (rect.get_height() == -1) rect = rect.new_with_size(rect.get_width(), pixrec.get_height());
    } else if (texture_type == ulx::object::TextureType::Vector) {
        auto svg_image = std::get<ulx::pair<ulx::file, ulx::rect>>(texture_variant);
        auto it = vector_texture_cache.find(svg_image);
        if (it != vector_texture_cache.end())
            texture_index = it->second;
        else {
            ulx::f32 width = svg_image.second.get_width();
            ulx::f32 height = svg_image.second.get_height();
            ulx::f32 physical_width = ulx::wfn::physical_cast(width, window_dpi);
            ulx::f32 physical_height = ulx::wfn::physical_cast(height, window_dpi);

            push_texture_image_cpu(ulx::pixmap::from_vector(svg_image.first, physical_width, physical_height));
            texture_index = texture_image_pixmaps.size() - 1;
            vector_texture_cache[svg_image] = texture_index;
        }

        ulx::pixmap pixmap = texture_image_pixmaps[texture_index];
        ulx::rect pixrec = pixmap.get_rect();
        if (rect.get_width() == -1) rect = rect.new_with_size(pixrec.get_width() / 2, rect.get_height());
        if (rect.get_height() == -1) rect = rect.new_with_size(rect.get_width(), pixrec.get_height() / 2);
    } else if (texture_type == ulx::object::TextureType::Text) {
        auto text_image = std::get<ulx::pair<ulx::str, ulx::font>>(texture_variant);
        auto it = font_texture_cache.find(text_image);
        if (it != font_texture_cache.end())
            texture_index = it->second;
        else {
            push_texture_image_cpu(ulx::pixmap::from_text(text_image.first, text_image.second));
            texture_index = texture_image_pixmaps.size() - 1;
            font_texture_cache[text_image] = texture_index;
        }

        ulx::pixmap pixmap = texture_image_pixmaps[texture_index];
        ulx::rect pixrec = pixmap.get_rect();
        ulx::font font = text_image.second;
        if (rect.get_width() == -1) rect = rect.new_with_size(pixrec.get_width() / font.get_scale(), rect.get_height());
        if (rect.get_height() == -1) rect = rect.new_with_size(rect.get_width(), pixrec.get_height() / font.get_scale());
    }
    rect = rect.new_with_size(rect.get_width() * object.get_scale(), rect.get_height() * object.get_scale());
}

inline auto UContext::push_object(const ulx::object& object, ulx::rect rect, ulx::u32& render_index, ulx::f32 window_height, ulx::u32 texture_index) -> void {
    ulx::color top_left_color = object.get_top_left_color();
    ulx::color top_right_color = object.get_top_right_color();
    ulx::color bottom_left_color = object.get_bottom_left_color();
    ulx::color bottom_right_color = object.get_bottom_right_color();
    ulx::color top_left_border_color = object.get_top_left_border_color();
    ulx::color top_right_border_color = object.get_top_right_border_color();
    ulx::color bottom_left_border_color = object.get_bottom_left_border_color();
    ulx::color bottom_right_border_color = object.get_bottom_right_border_color();
    float top_left_corner_radius = ulx::wfn::physical_cast(object.get_top_left_corner_radius(), window_dpi);
    float top_right_corner_radius = ulx::wfn::physical_cast(object.get_top_right_corner_radius(), window_dpi);
    float bottom_left_corner_radius = ulx::wfn::physical_cast(object.get_bottom_left_corner_radius(), window_dpi);
    float bottom_right_corner_radius = ulx::wfn::physical_cast(object.get_bottom_right_corner_radius(), window_dpi);
    float top_left_border_width = ulx::wfn::physical_cast(object.get_top_left_border_width(), window_dpi);
    float top_right_border_width = ulx::wfn::physical_cast(object.get_top_right_border_width(), window_dpi);
    float bottom_left_border_width = ulx::wfn::physical_cast(object.get_bottom_left_border_width(), window_dpi);
    float bottom_right_border_width = ulx::wfn::physical_cast(object.get_bottom_right_border_width(), window_dpi);
    float x = ulx::wfn::physical_cast(rect.get_x(), window_dpi);
    float y = ulx::wfn::physical_cast(rect.get_y(), window_dpi);
    float width = ulx::wfn::physical_cast(rect.get_width(), window_dpi);
    float height = ulx::wfn::physical_cast(rect.get_height(), window_dpi);
    glm::vec2 half_size = {width / 2, height / 2};
    glm::vec2 center = {x + half_size.x + 0.5f, window_height - (y + half_size.y) + 0.5f};
    glm::vec2 top_left = center - half_size;
    glm::vec2 top_right = {center.x + half_size.x, center.y - half_size.y};
    glm::vec2 bottom_right = center + half_size;
    glm::vec2 bottom_left = {center.x - half_size.x, center.y + half_size.y};
    glm::vec4 bottom_right_color_vec = __uii::vkalg::color_to_vec4(bottom_right_color);
    glm::vec4 bottom_left_color_vec = __uii::vkalg::color_to_vec4(bottom_left_color);
    glm::vec4 top_left_color_vec = __uii::vkalg::color_to_vec4(top_left_color);
    glm::vec4 top_right_color_vec = __uii::vkalg::color_to_vec4(top_right_color);
    glm::vec4 bottom_right_border_color_vec = __uii::vkalg::color_to_vec4(bottom_right_border_color);
    glm::vec4 bottom_left_border_color_vec = __uii::vkalg::color_to_vec4(bottom_left_border_color);
    glm::vec4 top_left_border_color_vec = __uii::vkalg::color_to_vec4(top_left_border_color);
    glm::vec4 top_right_border_color_vec = __uii::vkalg::color_to_vec4(top_right_border_color);

    render_vertices.insert(render_vertices.end(), {
        {top_right, bottom_right_color_vec, bottom_right_corner_radius, half_size, bottom_right_border_width, bottom_right_border_color_vec, center, {1.0f, 1.0f}, texture_index},
        {top_left, bottom_left_color_vec, bottom_left_corner_radius, half_size, bottom_left_border_width, bottom_left_border_color_vec, center, {0.0f, 1.0f}, texture_index},
        {bottom_left, top_left_color_vec, top_left_corner_radius, half_size, top_left_border_width, top_left_border_color_vec, center, {0.0f, 0.0f}, texture_index},
        {bottom_right, top_right_color_vec, top_right_corner_radius, half_size, top_right_border_width, top_right_border_color_vec, center, {1.0f, 0.0f}, texture_index}});
    render_indices.insert(render_indices.end(), {
        render_index, render_index + 1, render_index + 2,
        render_index + 2, render_index + 3, render_index});
    render_index += 4;
}

inline auto UContext::push_objects(const ulx::vec<ulx::object>& objects, ulx::u32& render_index, ulx::f32 window_height, const ulx::rect& parent_rect) -> void {
    for (auto& object : objects) {
        ulx::rect rect = object.get_rect(); ulx::u32 texture_index;
        update_texture_images(rect, object, texture_index);
        if (rect.get_width() == -2) rect = rect.new_with_size(parent_rect.get_width(), rect.get_height());
        if (rect.get_height() == -2) rect = rect.new_with_size(rect.get_width(), parent_rect.get_height());
        rect = rect.new_with_pos(rect.get_x() + parent_rect.get_x(), rect.get_y() + parent_rect.get_y());
        push_object(object, rect, render_index, window_height, texture_index);

        ulx::vec<ulx::object> child_objects = object.get_objects();
        push_objects(child_objects, render_index, window_height, rect);
    }
}

inline auto UContext::update_scene_objects() -> void {
    ulx::u32 render_index = 0;
    render_vertices.clear(); render_indices.clear();
    current_render_scene = render_callback(*this);
    ulx::vec<ulx::object> objects = current_render_scene.get_objects();
    current_background_color = current_render_scene.get_background_color();

    push_objects(objects, render_index, static_cast<ulx::f32>(window_current_height),
        ulx::rect(0, 0, ulx::wfn::logical_cast(window_current_width, window_dpi),
                    ulx::wfn::logical_cast(window_current_height, window_dpi)
        )
    );

    if (push_texture_images_to_gpu)
        push_texture_images_gpu();
}

inline auto UContext::set_timer(const TickTimer& timer) -> void { this->tick_timer = timer; }
inline auto UContext::dirtied(bool data_dirty) -> void { render_dirty = true; this->data_dirty = data_dirty; }
inline auto UContext::titled() const -> bool { return window_titled_state; }
inline auto UContext::resizable() const -> bool { return window_resizable_state; }
inline auto UContext::dpi() const -> UINT { return window_dpi; }

inline auto UContext::window_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT {
    switch (message) {
        case WM_NCCREATE: {
            auto* create_structure = reinterpret_cast<CREATESTRUCTW*>(lparam);
            auto* ctx = static_cast<UContext*>(create_structure->lpCreateParams);

            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
            return DefWindowProcW(hwnd, message, wparam, lparam);
        } case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } case WM_NCDESTROY: {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
            return DefWindowProcW(hwnd, message, wparam, lparam);
        } case WM_NCHITTEST: {
            auto* ctx = reinterpret_cast<UContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);
            if (!ctx->resizable() || IsZoomed(hwnd)) return HTCLIENT;

            POINT cursor_position; GetCursorPos(&cursor_position);
            RECT window_rect; GetWindowRect(hwnd, &window_rect);
            int frame = GetSystemMetricsForDpi(SM_CYFRAME, ctx->dpi());

            // Top edge
            if (cursor_position.y >= window_rect.top - frame && cursor_position.y < window_rect.top + frame)
                return HTTOP;

            return DefWindowProcW(hwnd, message, wparam, lparam); // Another edges and "corner" cases
        } case WM_NCCALCSIZE: {
            auto* ctx = reinterpret_cast<UContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx || ctx->titled()) return DefWindowProcW(hwnd, message, wparam, lparam);

            /* Move the top of the client area to the top of the window
                to cover the titlebar */
            auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
            params->rgrc[0].top -= ctx->window_titlebar_height;

            return DefWindowProcW(hwnd, WM_NCCALCSIZE, wparam, lparam);
        } case WM_PAINT: {
            auto* ctx = reinterpret_cast<UContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            ctx->draw_frame();

            ValidateRect(hwnd, nullptr);
            return 0;
        } case WM_GETMINMAXINFO: {
            auto* ctx = reinterpret_cast<UContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            ulx::rect min_rect = ctx->min_rect();
            ulx::rect max_rect = ctx->max_rect();
            min_rect = min_rect.new_with_size(ulx::wfn::logical_cast(min_rect.get_width()), ulx::wfn::logical_cast(min_rect.get_height()));
            max_rect = max_rect.new_with_size(ulx::wfn::logical_cast(max_rect.get_width()), ulx::wfn::logical_cast(max_rect.get_height()));

            MINMAXINFO* minmax_info = reinterpret_cast<MINMAXINFO*>(lparam);
            minmax_info->ptMinTrackSize.x = min_rect.get_width();
            minmax_info->ptMinTrackSize.y = min_rect.get_height();
            minmax_info->ptMaxTrackSize.x = max_rect.get_width();
            minmax_info->ptMaxTrackSize.y = max_rect.get_height();

            return 0;
        } case WM_SIZE: {
            auto* ctx = reinterpret_cast<UContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
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
            auto* ctx = reinterpret_cast<UContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            RECT rect; GetClientRect(hwnd, &rect);
            ctx->window_current_height = rect.bottom - rect.top;
            ctx->window_current_width = rect.right - rect.left;
            ctx->update_push_constant();

            ctx->dirtied(true);
            ctx->draw_frame();

            return 0;
        } case WM_EXITSIZEMOVE: {
            auto* ctx = reinterpret_cast<UContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
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

inline auto UContext::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] auto* user_data) -> VkBool32 {
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            ulx::log::ulixwarn(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            ulx::log::ulixerr(callback_data->pMessage);
        default: break;
    }

    return VK_FALSE;
}

inline auto UContext::create_window(const ulx::wininfo& window_info) -> void {
    ulx::str window_class_name = window_info.get_window_class_name();
    ulx::str window_title = window_info.get_window_title();
    ulx::wstr window_wide_class_name = ulx::wstr(window_class_name.begin(), window_class_name.end());
    ulx::wstr window_wide_title = ulx::wstr(window_title.begin(), window_title.end());
    ulx::rect window_rect = window_info.get_initial_window_rect();
    window_min_rect = window_info.get_min_window_rect();
    window_max_rect = window_info.get_max_window_rect();
    if (window_rect.get_width() > window_min_rect.get_width())
        window_rect = window_min_rect.new_with_size(window_rect.get_width(), window_rect.get_height());
    if (window_rect.get_height() > window_min_rect.get_height())
        window_rect = window_min_rect.new_with_size(window_rect.get_width(), window_rect.get_height());
    if (window_rect.get_width() > window_max_rect.get_width())
        window_rect = window_max_rect.new_with_size(window_rect.get_width(), window_rect.get_height());
    if (window_rect.get_height() > window_max_rect.get_height())
        window_rect = window_max_rect.new_with_size(window_rect.get_width(), window_rect.get_height());
    ulx::u32 physical_window_width = ulx::wfn::physical_cast(window_rect.get_width());
    ulx::u32 physical_window_height = ulx::wfn::physical_cast(window_rect.get_height());
    ulx::i32 physical_window_x = ulx::wfn::physical_cast(window_rect.get_x());
    ulx::i32 physical_window_y = ulx::wfn::physical_cast(window_rect.get_y());

    DWORD window_style = WS_OVERLAPPED |  WS_CAPTION |  WS_THICKFRAME |  WS_MINIMIZEBOX |  WS_MAXIMIZEBOX;
    if (window_info.get_window_attrs() & ulx::winattr::Titled) {
        window_titled_state = true;
    } else if (!(window_info.get_window_attrs() & ulx::winattr::Bordered)) {
        window_style &= ~WS_BORDER;
        window_resizable_state = false;
    }

    if (window_info.get_window_attrs() & ulx::winattr::Resizable) {
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
    if (!RegisterClassExW(&wcex))
        ulx::log::ulixerr("failed to register window class, error code: {}", GetLastError());

    window_hwnd = CreateWindowExW(
        0, window_wide_class_name.c_str(), window_wide_title.c_str(),
        window_style, physical_window_x, physical_window_y, physical_window_width, physical_window_height,
        nullptr, nullptr, ulx::wfn::hinstance, this);
    if (!window_hwnd) ulx::log::ulixerr("failed to create window, error code: {}", GetLastError());

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

inline UContext::UContext(const ulx::appinfo& application_info, const ulx::wininfo& window_info, const ulx::renderinfo& render_info) {
    create_window(window_info);
    create_vulkan_objects(application_info, render_info);
}
inline UContext::~UContext() { destroy_vulkan_objects(); }

inline auto UContext::exec() -> int {
    MSG msg{}; while (true) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return msg.wParam;
            DispatchMessageW(&msg);
        } else if (render_dirty) draw_frame();
        else WaitMessage();
    }

    return 0;
}

inline auto UContext::min_rect() const -> ulx::rect { return window_min_rect; }
inline auto UContext::set_min_rect(const ulx::rect& rect) -> void { window_min_rect = rect; }
inline auto UContext::max_rect() const -> ulx::rect { return window_max_rect; }
inline auto UContext::set_max_rect(const ulx::rect& rect) -> void { window_max_rect = rect; }
inline auto UContext::show() -> void { ShowWindow(window_hwnd, SW_SHOW); UpdateWindow(window_hwnd); }
inline auto UContext::hide() -> void { ShowWindow(window_hwnd, SW_HIDE); UpdateWindow(window_hwnd); }
inline auto UContext::exit() -> void { PostMessageW(window_hwnd, WM_CLOSE, 0, 0); }
inline auto UContext::rect() -> ulx::rect {
    RECT window_rect{}; GetWindowRect(window_hwnd, &window_rect);
    return ulx::rect(window_rect.left, window_rect.top,
                 ulx::wfn::logical_cast(window_rect.right - window_rect.left),
                 ulx::wfn::logical_cast(window_rect.bottom - window_rect.top));
}
inline auto UContext::set_rect(const ulx::rect& rect) -> void {
    ulx::i32 physical_x = ulx::wfn::physical_cast(rect.get_x());
    ulx::i32 physical_y = ulx::wfn::physical_cast(rect.get_y());
    ulx::u32 physical_width = ulx::wfn::physical_cast(rect.get_width());
    ulx::u32 physical_height = ulx::wfn::physical_cast(rect.get_height());

    MoveWindow(window_hwnd, physical_x, physical_y, physical_width, physical_height, TRUE);
}
inline auto UContext::rendering() const -> bool { return rending; }

inline auto UContext::init() -> void {
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        ulx::log::ulixerr("failed to set DPI awareness, error code: {}", GetLastError());
}

inline auto UContext::create_vulkan_instance() -> void {
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

    if (enabled_debug) {
        if (!check_validation_layer_support())
            ulx::log::ulixerr("requested validation layers not available");

        vulkan_instance_create_info.enabledLayerCount++;
        vulkan_instance_create_info.enabledExtensionCount++;

        debug_utils_messenger_create_info = {};
        debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_utils_messenger_create_info.pfnUserCallback = debug_callback;
        vulkan_instance_create_info.pNext = &debug_utils_messenger_create_info;
    }

    switch (vkCreateInstance(&vulkan_instance_create_info, VK_NULL_HANDLE, &vulkan_instance)) {
        case VK_SUCCESS: return;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            ulx::log::ulixerr("failed to create vulkan instance: Incompatible driver");
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            ulx::log::ulixerr("failed to create vulkan instance: Extension not present");
        default:
            ulx::log::ulixerr("failed to create vulkan instance: Unknown type error");
    }
}

inline auto UContext::check_validation_layer_support() -> bool {
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

inline auto UContext::create_debug_messenger() -> void {
    if (!enabled_debug) return;

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info;
    populate_debug_messenger_create_info(debug_utils_messenger_create_info);

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func == VK_NULL_HANDLE)
        ulx::log::ulixerr("failed to get function pointer vkCreateDebugUtilsMessengerEXT from Vulkan instance");
    if (func(vulkan_instance, &debug_utils_messenger_create_info, VK_NULL_HANDLE, &debug_messenger) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create debug messenger");
}

inline auto UContext::destroy_debug_utils_messenger() -> void {
    if (debug_messenger == VK_NULL_HANDLE)
        return;

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vulkan_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func == VK_NULL_HANDLE)
        ulx::log::ulixerr("failed to get function pointer vkDestroyDebugUtilsMessengerEXT from Vulkan instance");
    func(vulkan_instance, debug_messenger, VK_NULL_HANDLE);
}

inline auto UContext::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) -> void {
    debug_utils_messenger_create_info = {};
    debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_utils_messenger_create_info.pfnUserCallback = debug_callback;
}

inline auto UContext::select_physical_device() -> void {
    ulx::u32 physical_device_count = 0;
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, VK_NULL_HANDLE);
    if (physical_device_count == 0) ulx::log::ulixerr("failed to find GPUs with Vulkan support");
    ulx::vec<VkPhysicalDevice> physical_devices = ulx::vec<VkPhysicalDevice>(physical_device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, physical_devices.data());

    VkPhysicalDevice alternative_device = VK_NULL_HANDLE;
    __uii::vkclses::PhysicalDeviceInfos alternative_device_infos;
    ulx::u32 last_discrete_gpu_score = 0;
    ulx::u32 last_alternative_device_score = 0;

    for (const auto& device : physical_devices) {
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
    if (alternative_device == VK_NULL_HANDLE) ulx::log::ulixerr("failed to pick a suitable GPU");
    physical_device = alternative_device;
    physical_device_infos = alternative_device_infos;
}

inline auto UContext::create_logical_device() -> void {
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
    if (vkCreateDevice(physical_device, &device_create_info, VK_NULL_HANDLE, &logical_device) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create logical device");

    device_queues = __uii::vkclses::DeviceQueues(logical_device, physical_device_infos);
}

inline auto UContext::create_window_surface() -> void {
    VkWin32SurfaceCreateInfoKHR win32_surface_create_info{};
    win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32_surface_create_info.hwnd = window_hwnd;
    win32_surface_create_info.hinstance = ulx::wfn::hinstance;

    if (vkCreateWin32SurfaceKHR(vulkan_instance, &win32_surface_create_info, VK_NULL_HANDLE, &window_surface) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create win32 window surface");
}

inline auto UContext::create_swapchain() -> void {
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
    if (vkCreateSwapchainKHR(logical_device, &swapchain_create_info, VK_NULL_HANDLE, &swapchain) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create swapchain");

    swapchain_image_format = surface_format.format;
    swapchain_extent = extent;

    get_swapchain_images();
}

inline auto UContext::get_swapchain_images() -> void {
    ulx::u32 image_count = 0;
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, VK_NULL_HANDLE);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, swapchain_images.data());

    max_frames_in_flight = static_cast<ulx::size>(image_count);
}

inline auto UContext::create_swapchain_image_views() -> void {
    swapchain_image_views.resize(swapchain_images.size());

    for (ulx::size index = 0; index < swapchain_images.size(); index++)
        swapchain_image_views[index] = __uii::vkalg::create_image_view(logical_device, swapchain_images[index], swapchain_image_format);
}

inline auto UContext::create_shader_module(unsigned char* code, unsigned int length) -> VkShaderModule {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = length;
    shader_module_create_info.pCode = reinterpret_cast<ulx::u32*>(code);

    VkShaderModule shader_module;
    if (vkCreateShaderModule(logical_device, &shader_module_create_info, VK_NULL_HANDLE, &shader_module) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create shader module");

    return shader_module;
}

inline auto UContext::populate_pipeline_shader_stage_create_info(VkPipelineShaderStageCreateInfo& create_info, VkShaderModule shader_module, VkShaderStageFlagBits stage) -> void {
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage = stage;
    create_info.module = shader_module;
    create_info.pName = "main";
}

inline auto UContext::create_graphics_pipeline() -> void {
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

    VkVertexInputBindingDescription vertex_input_binding_description = __uii::vsdces::Vertex2D::get_binding_description();
    ulx::vec<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions = __uii::vsdces::Vertex2D::get_attribute_descriptions();

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
    if (vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, VK_NULL_HANDLE, &pipeline_layout) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create pipeline layout");


    __uii::vkalg::ensure_cache_exists(cache_file_path);
    ulx::vec<char> cache_data = __uii::vkalg::read_cache(cache_file_path);
    VkPipelineCacheCreateInfo pipeline_cache_create_info{};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipeline_cache_create_info.initialDataSize = cache_data.size();
    pipeline_cache_create_info.pInitialData = cache_data.data();
    VkPipelineCache pipeline_cache;
    if (vkCreatePipelineCache(logical_device, &pipeline_cache_create_info, nullptr, &pipeline_cache) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create pipeline cache");

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
    if (vkCreateGraphicsPipelines(logical_device, pipeline_cache, 1, &graphics_pipeline_create_info, VK_NULL_HANDLE, &graphics_pipeline) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create graphics pipeline");


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

inline auto UContext::create_render_pass() -> void {
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
    if (vkCreateRenderPass(logical_device, &render_pass_create_info, VK_NULL_HANDLE, &render_pass) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create render pass");
}

inline auto UContext::create_swapchain_frame_buffers() -> void {
    swapchain_frame_buffers.resize(swapchain_images.size());

    for (ulx::size index = 0; index < swapchain_images.size(); index++) {
        VkFramebufferCreateInfo frame_buffer_create_info{};
        frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_create_info.renderPass = render_pass;
        frame_buffer_create_info.attachmentCount = 1;
        frame_buffer_create_info.pAttachments = &swapchain_image_views[index];
        frame_buffer_create_info.width = swapchain_extent.width;
        frame_buffer_create_info.height = swapchain_extent.height;
        frame_buffer_create_info.layers = 1;
        if (vkCreateFramebuffer(logical_device, &frame_buffer_create_info, VK_NULL_HANDLE, &swapchain_frame_buffers[index]) != VK_SUCCESS)
            ulx::log::ulixerr("failed to create frame buffers");
    }
}

inline auto UContext::create_command_pool() -> void {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = physical_device_infos.queue_family_indices.graphics_queue_family_index.value();
    if (vkCreateCommandPool(logical_device, &command_pool_create_info, VK_NULL_HANDLE, &command_pool) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create command pool");
}

inline auto UContext::create_command_buffers() -> void {
    command_buffers.resize(max_frames_in_flight);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = static_cast<ulx::u32>(command_buffers.size());
    if (vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, command_buffers.data()) != VK_SUCCESS)
        ulx::log::ulixerr("failed to allocate command buffer");
}

inline auto UContext::create_sync_objects() -> void {
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
            ulx::log::ulixerr("failed to create sync object: image available semaphore");
        if (vkCreateSemaphore(logical_device, &semaphore_create_info, VK_NULL_HANDLE, &render_finished_semaphores[index]) != VK_SUCCESS)
            ulx::log::ulixerr("failed to create sync object: render finished semaphore");
        if (vkCreateFence(logical_device, &fence_create_info, VK_NULL_HANDLE, &in_flight_fences[index]) != VK_SUCCESS)
            ulx::log::ulixerr("failed to create sync object: in flight fence");
    }
}

inline auto UContext::destroy_swapchain() -> void {
    for (const auto& frame_buffer : swapchain_frame_buffers)
        vkDestroyFramebuffer(logical_device, frame_buffer, VK_NULL_HANDLE);
    for (const auto& image_view : swapchain_image_views)
        vkDestroyImageView(logical_device, image_view, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(logical_device, swapchain, VK_NULL_HANDLE);
}

inline auto UContext::recreate_swapchain() -> void {
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

inline auto UContext::create_staging_buffer() -> void {
    VkDeviceSize texture_size = create_texture_images();
    VkDeviceSize vertex_size = render_vertices.size() * sizeof(__uii::vsdces::Vertex2D);
    VkDeviceSize index_size = render_indices.size() * sizeof(ulx::u32);

    staging_buffer_size = std::max(std::max(vertex_size, index_size), texture_size);
    staging_buffer = __uii::vkclses::Buffer(
        physical_device, logical_device, staging_buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
}

inline auto UContext::create_vertex_buffer() -> void {
    VkDeviceSize size = render_vertices.size() * sizeof(__uii::vsdces::Vertex2D);
    vertex_buffer = __uii::vkclses::Buffer(
        physical_device, logical_device, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
}

inline auto UContext::create_index_buffer() -> void {
    VkDeviceSize size = render_indices.size() * sizeof(ulx::u32);
    index_buffer = __uii::vkclses::Buffer(
        physical_device, logical_device, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
}

inline auto UContext::update_object_buffers() -> void {
    staging_buffer.map_memory(logical_device, render_vertices.data(), render_vertices.size() * sizeof(__uii::vsdces::Vertex2D));
    staging_buffer.copy_buffer_to(vertex_buffer, logical_device, command_pool, device_queues.graphics_queue, render_vertices.size() * sizeof(__uii::vsdces::Vertex2D));
    staging_buffer.map_memory(logical_device, render_indices.data(), render_indices.size() * sizeof(ulx::u32));
    staging_buffer.copy_buffer_to(index_buffer, logical_device, command_pool, device_queues.graphics_queue, render_indices.size() * sizeof(ulx::u32));
}

inline auto UContext::record_command_buffer(VkCommandBuffer command_buffer, ulx::u32 image_index) -> void {
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = VK_NULL_HANDLE;
    if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS)
        ulx::log::ulixerr("failed to begin command buffer");

    VkClearValue clear_color = {{{current_background_color.get_red() / 255.0f, current_background_color.get_green() / 255.0f, current_background_color.get_blue() / 255.0f, current_background_color.get_alpha() / 255.0f}}};
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
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        ulx::log::ulixerr("failed to end command buffer");
}

inline auto UContext::draw_frame() -> void {
    rending = true;
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
    } else if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)
        ulx::log::ulixerr("failed to acquire next image");

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
    if (vkQueueSubmit(device_queues.graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS)
        ulx::log::ulixerr("failed to submit queue");

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
    rending = false;
}

inline auto UContext::create_descriptor_set_layout() -> void {
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
    if (vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout))
        ulx::log::ulixerr("failed to create descriptor set layout");
}

inline auto UContext::create_descriptor_pool() -> void {
    VkDescriptorPoolSize descriptor_pool_size{};
    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_pool_size.descriptorCount = max_texture_count;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;
    descriptor_pool_create_info.maxSets = 1;
    if (vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &descriptor_pool))
        ulx::log::ulixerr("failed to create descriptor pool");
}

inline auto UContext::create_descriptor_set() -> void {
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.descriptorPool = descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = 1;
    descriptor_set_allocate_info.pSetLayouts = &descriptor_set_layout;
    if (vkAllocateDescriptorSets(logical_device, &descriptor_set_allocate_info, &descriptor_set))
        ulx::log::ulixerr("failed to allocate descriptor sets");


    ulx::vec<VkDescriptorImageInfo> descriptor_image_infos(texture_image_pixmaps.size());
    for (ulx::size index = 0; index < texture_image_pixmaps.size(); index++) {
        descriptor_image_infos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptor_image_infos[index].imageView = texture_images[index].image_view;
        descriptor_image_infos[index].sampler = texture_image_sampler;
    }

    VkWriteDescriptorSet descriptor_set_write{};
    descriptor_set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_set_write.dstSet = descriptor_set;
    descriptor_set_write.dstBinding = 0;
    descriptor_set_write.dstArrayElement = 0;
    descriptor_set_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_set_write.descriptorCount = static_cast<ulx::u32>(texture_image_pixmaps.size());
    descriptor_set_write.pImageInfo = descriptor_image_infos.data();
    vkUpdateDescriptorSets(logical_device, 1, &descriptor_set_write, 0, nullptr);
}

inline auto UContext::map_texture_images() -> void {
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
    for (auto& pixmap : texture_image_pixmaps) {
        staging_total_size += pixmap.get_pixel_size();
    }

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = current_offset;
    memory_allocate_info.memoryTypeIndex = __uii::vkalg::find_memory_type(physical_device, memory_type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, &texture_image_memory))
        ulx::log::ulixerr("failed to allocate texture image memory");


    for (ulx::size index = 0; index < texture_image_pixmaps.size(); index++)
        vkBindImageMemory(logical_device, texture_images[index].image, texture_image_memory, offsets[index]);

    void* mapped; VkDeviceSize staging_offset = 0;
    vkMapMemory(logical_device, staging_buffer.buffer_memory, 0, staging_total_size, 0, &mapped);
    VkCommandBuffer single_time_command_buffer = __uii::vkalg::start_single_time_command_buffer(logical_device, command_pool);
    for (ulx::size index = 0; index < texture_image_pixmaps.size(); index++) {
        VkImage image = texture_images[index].image;
        ulx::pixmap pixmap = texture_image_pixmaps[index];
        ulx::rect rect = pixmap.get_rect();

        memcpy((char*)mapped + staging_offset, pixmap.get_pixels().data(), pixmap.get_pixel_size());
        offsets[index] = staging_offset;
        staging_offset += pixmap.get_pixel_size();

        __uii::vkalg::transition_image_layout(logical_device, single_time_command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        __uii::vkalg::copy_buffer_to_image(logical_device, staging_buffer.buffer, single_time_command_buffer, image, rect.get_width(), rect.get_height(), offsets[index]);
        __uii::vkalg::transition_image_layout(logical_device, single_time_command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    __uii::vkalg::end_single_time_command_buffer(logical_device, device_queues.graphics_queue, command_pool, single_time_command_buffer);
    vkUnmapMemory(logical_device, staging_buffer.buffer_memory);
}

inline auto UContext::create_texture_image(ulx::size index) -> VkMemoryRequirements {
    VkImage& texture = texture_images[index].image;
    ulx::pixmap pixmap = texture_image_pixmaps[index];
    ulx::rect rect = pixmap.get_rect();

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = rect.get_width();
    image_create_info.extent.height = rect.get_height();
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (vkCreateImage(logical_device, &image_create_info, VK_NULL_HANDLE, &texture) != VK_SUCCESS)
        ulx::log::ulixerr("failed to create texture image");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(logical_device, texture, &memory_requirements);
    return memory_requirements;
}

inline auto UContext::create_texture_image_views() -> void {
    for (ulx::size index = 0; index < texture_images.size(); ++index)
        texture_images[index].image_view = __uii::vkalg::create_image_view(logical_device, texture_images[index].image, VK_FORMAT_R8G8B8A8_UNORM);
}

inline auto UContext::create_texture_image_sampler() -> void {
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
    if (vkCreateSampler(logical_device, &sampler_create_info, nullptr, &texture_image_sampler))
        ulx::log::ulixerr("failed to create texture image sampler");
}

inline auto UContext::push_texture_image_cpu(const ulx::pixmap& pixmap) -> void {
    texture_image_pixmaps.push_back(pixmap);
    push_texture_images_to_gpu = true;
}

inline auto UContext::push_texture_images_gpu() -> void {
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

inline auto UContext::recreate_staging_buffer() -> void {
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

inline auto UContext::create_texture_images() -> VkDeviceSize {
    VkDeviceSize size = 0; texture_images.resize(texture_image_pixmaps.size());
    for (ulx::size index = 0; index < texture_image_pixmaps.size(); index++) {
        VkMemoryRequirements memory_requirements = create_texture_image(index);
        size += memory_requirements.size;
    }

    return size;
}

inline auto UContext::destroy_vulkan_objects() -> void {
    vkDeviceWaitIdle(logical_device);

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

inline auto UContext::create_vulkan_objects(const ulx::appinfo& application_info, const ulx::renderinfo& render_info) -> void {
    render_callback = reinterpret_cast<RenderCallback>(render_info.get_render_callback());
    texture_image_pixmaps = { __uii::vkalg::empty_pixmap() };
    texture_index_max = texture_image_pixmaps.size();
    enabled_debug = application_info.get_enabled_vulkan_debug();
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
