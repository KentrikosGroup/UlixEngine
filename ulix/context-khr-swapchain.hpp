#pragma once
#include "__class_decl/context.hpp"
#include "__inside_impl/atlas_texture_merger.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "stablestr.hpp"
#include "ulix/__required_libraries/rapidjson/rapidjson.h"
#include "ulix/font.hpp"
#include "vulkan/vulkan_core.h"
#include <future>
#include <mutex>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#include "align.hpp"
#include "cursor.hpp"
#include "box.hpp"
#include "pixmap.hpp"
#include "point.hpp"
#include "size.hpp"
#include "renderinfo.hpp"
#include "scene.hpp"
#include "log.hpp"
#include "color.hpp"
#include "element.hpp"
#include "wininfo.hpp"
#include "appinfo.hpp"
#include "winfunc.hpp"
#include "types.hpp"
#include "__inside_impl/vulkan_classes.hpp"
#include "__inside_impl/vulkan_shader_data_classes.hpp"
#include "__inside_impl/vulkan_algorithm.hpp"
#include "__inside_impl/vulkan_requirements.hpp"
#include "__builtin_shaders/fragment_shader.inl"
#include "__builtin_shaders/vertex_shader.inl"
#include "__required_libraries/rapidjson/document.h"
#include "__required_libraries/rapidjson/prettywriter.h"
#include <vulkan/vulkan.h>
#include <windows.h>
#undef GetObject


inline auto ulx::context::update_push_constant() -> void {
    m_pushconstant.projection = glm::ortho(
        0.0f, static_cast<ulx::f32>(m_mainwin_width),
        static_cast<ulx::f32>(m_mainwin_height), 0.0f,
        -1.0f, 1.0f);
}

inline auto ulx::context::update_texture_images(ulx::size& siz, const element& object, u32& texture_index) -> __uii::ats::uv {
    const element::texture::variant& texture_variant = object.get_texture();
    __uii::ats::uv resuv = {{0, 0}, {1, 1}};

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::same_as<T, element::texture::none>) {
            texture_index = 0;
        } else if constexpr (std::same_as<T, element::texture::bitmap>) {
            auto it = m_bitmap_cache.find(arg);
            if (it != m_bitmap_cache.end()) {
                texture_index = it->second.template get<0>();
                resuv = it->second.template get<1>();
            } else {
                pixmap pm = pixmap::from_bitmap(arg);
                __uii::ats::idx idx = __uii::ats::push_pixmap(m_texture_image_pixmaps, m_atlas_index, m_atlas_x, m_atlas_y, m_atlas_height, pm);
                texture_index = idx.template get<0>();
                resuv = idx.template get<1>();
                m_bitmap_cache[arg] = idx;
                m_texture_pushtag = true;
            }

            ulx::size uvsiz = resuv.template get<1>();
            if (siz.width() == -1) siz = {wfn::logical(uvsiz.width() * 2048, m_mainwin_dpi), siz.height()};
            if (siz.height() == -1) siz = {siz.width(), wfn::logical(uvsiz.height() * 2048, m_mainwin_dpi)};
        } else if constexpr (std::same_as<T, element::texture::vector>) {
            auto it = m_vector_cache.find(arg);
            if (it != m_vector_cache.end()) {
                texture_index = it->second.template get<0>();
                resuv = it->second.template get<1>();
            } else {
                f32 width = arg.template get<1>().width();
                f32 height = arg.template get<1>().height();
                f32 physical_width = wfn::physical(width, m_mainwin_dpi);
                f32 physical_height = wfn::physical(height, m_mainwin_dpi);

                pixmap pm = pixmap::from_vector(arg.template get<0>(), physical_width, physical_height);
                __uii::ats::idx idx = __uii::ats::push_pixmap(m_texture_image_pixmaps, m_atlas_index, m_atlas_x, m_atlas_y, m_atlas_height, pm);
                texture_index = idx.template get<0>();
                resuv = idx.template get<1>();
                m_vector_cache[arg] = idx;
                m_texture_pushtag = true;
            }

            ulx::size uvsiz = resuv.template get<1>();
            if (siz.width() == -1) siz = {wfn::logical(uvsiz.width() * 1024, m_mainwin_dpi), siz.height()};
            if (siz.height() == -1) siz = {siz.width(), wfn::logical(uvsiz.height() * 1024, m_mainwin_dpi)};
        } else if constexpr (std::same_as<T, element::texture::text>) {
            auto it = m_font_cache.find(arg);
            if (it != m_font_cache.end()) {
                texture_index = it->second.template get<0>();
                resuv = it->second.template get<1>();
            } else {
                font font = arg.template get<1>();
                font.size(wfn::physical(font.get_size(), m_mainwin_dpi));
                pixmap pm = pixmap::from_char(arg.template get<0>(), font);
                __uii::ats::idx idx = __uii::ats::push_pixmap(m_texture_image_pixmaps, m_atlas_index, m_atlas_x, m_atlas_y, m_atlas_height, pm);
                texture_index = idx.template get<0>();
                resuv = idx.template get<1>();
                m_font_cache[arg] = idx;
                m_texture_pushtag = true;
            }

            ulx::size uvsiz = resuv.template get<1>();
            if (siz.width() == -1) siz = {wfn::logical(uvsiz.width() * 2048, m_mainwin_dpi), siz.height()};
            if (siz.height() == -1) siz = {siz.width(), wfn::logical(uvsiz.height() * 2048, m_mainwin_dpi)};
        }
    }, texture_variant);
    return resuv;
}

inline auto ulx::context::push_element(
    const element& elem, 
    const size& siz, const point& pos, 
    u32& render_index, u32 texture_index, 
    __uii::ats::uv& uv
) -> void {
    const color& background_color = elem.get_background_color();
    const color& border_color = elem.get_border_color();
    f32 corner_radius = wfn::physical(elem.get_corner_radius(), m_mainwin_dpi);
    f32 border_width = wfn::physical(elem.get_border_width(), m_mainwin_dpi);
    f32 x = wfn::physical(pos.x(), m_mainwin_dpi);
    f32 y = wfn::physical(pos.y(), m_mainwin_dpi);
    f32 width = wfn::physical(siz.width(), m_mainwin_dpi);
    f32 height = wfn::physical(siz.height(), m_mainwin_dpi);
    glm::vec2 half_size = {width / 2.0f, height / 2.0f};
    glm::vec2 center = {x + half_size.x + 0.5f, m_mainwin_height - (y + half_size.y) + 0.5f};
    glm::vec2 top_left = center - half_size;
    glm::vec2 top_right = {center.x + half_size.x, center.y - half_size.y};
    glm::vec2 bottom_right = center + half_size;
    glm::vec2 bottom_left = {center.x - half_size.x, center.y + half_size.y};
    glm::vec4 background_color_vec = __uii::vkalg::color_to_vec4(background_color);
    glm::vec4 border_color_vec = __uii::vkalg::color_to_vec4(border_color);

    point& uvpos = uv.get<0>();
    f32 uvx = uvpos.x(), uvy = uvpos.y();
    ulx::size& uvsiz = uv.get<1>();
    f32 uvw = uvsiz.width(), uvh = uvsiz.height();

    m_vertices.insert(m_vertices.end(), {
        {top_right,    background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {uvx + uvw, uvy + uvh}, texture_index},
        {top_left,     background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {uvx, uvy + uvh},       texture_index},
        {bottom_left,  background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {uvx, uvy},             texture_index},
        {bottom_right, background_color_vec, corner_radius, half_size, border_width, border_color_vec, center, {uvx + uvw, uvy},       texture_index}});
    m_indices.insert(m_indices.end(), {
        render_index, render_index + 1, render_index + 2,
        render_index + 2, render_index + 3, render_index});
    render_index += 4;
}

inline auto ulx::context::compute_layout(
    size& siz, point& pos, 
    const size& parent_size, const point& parent_pos, 
    box parent_layout, 
    f32& x_offset, f32& y_offset, 
    f32 padding, u8 align
) -> void {
    if (parent_layout == nonebox) return;
    else if (parent_layout == horbox) {
        if (align & alignleft) {
            pos = {x_offset + pos.x(), pos.y()};
            x_offset += padding + siz.width();
        } else if (align & alignright) {
            pos = {parent_size.width() - x_offset - siz.width(), pos.y()};
            x_offset += padding + siz.width();
        }

        if (align & alignbottom) {
            pos = {pos.x(), parent_size.height() + pos.y() - siz.height()};
        }
    } else if (parent_layout == verbox) {
        if (align & aligntop) {
            pos = {pos.x(), y_offset + pos.y()};
            y_offset += padding + siz.height();
        } else if (align & alignbottom) {
            pos = {pos.x(), parent_size.height() - y_offset - siz.height()};
            y_offset += padding + siz.height();
        }

        if (align & alignright) {
            pos = {parent_size.width() + pos.x() - siz.width(), pos.y()};
        }
    }
}

inline auto ulx::context::event_valid(
    const element::event::pressed& pressed_event,
    const element::event::released& released_event,
    const element::event::hovered& hovered_event
) -> bool {
    return pressed_event.valid() || released_event.valid() || hovered_event.valid();
}

inline auto ulx::context::push_elements(
    vec<element>& elements, u32& render_index, 
    const size& parent_size, const point& parent_pos, 
    box parent_layout, f32 parent_padding, 
    f32& x_offset, f32& y_offset, u8 align, 
    POINT& cursor_pt, HWND pt_hwnd, int left_mouse_state
) -> void {
    if (elements.empty()) return;
    for (auto& elem : elements) {
        ulx::size siz = elem.get_size();
        point pos = elem.get_pos();
        u32 texture_index;
        __uii::ats::uv uv = update_texture_images(siz, elem, texture_index);
        
        if (siz.width() == -2) siz = {parent_size.width(), siz.height()};
        if (siz.height() == -2) siz = {siz.width(), parent_size.height()};
        pos = {pos.x() + parent_pos.x(), pos.y() + parent_pos.y()};
        compute_layout(siz, pos, parent_size, parent_pos, parent_layout, x_offset, y_offset, parent_padding, align);

        if (siz.width() != 0 && siz.height() != 0) {
            const element::event::pressed& pressed_event = elem.get_pressed_event();
            const element::event::released& released_event = elem.get_released_event();
            const element::event::hovered& hovered_event = elem.get_hovered_event();
            
            if (pt_hwnd == m_mainwin || pt_hwnd == m_snapwin) {
                if (event_valid(pressed_event, released_event, hovered_event) &&
                    cursor_pt.x >= wfn::physical(pos.x(), m_mainwin_dpi) &&
                    cursor_pt.x <= wfn::physical(pos.x() + siz.width(), m_mainwin_dpi) &&

                    cursor_pt.y >= wfn::physical(pos.y(), m_mainwin_dpi) &&
                    cursor_pt.y <= wfn::physical(pos.y() + siz.height(), m_mainwin_dpi)
                ) {
                    if (left_mouse_state & 0x8000) {
                        if (pressed_event.valid()) pressed_event(*this, elem);
                    } else if (left_mouse_state & 0x0001) {
                        if (released_event.valid()) released_event(*this, elem);
                    } else {
                        if (hovered_event.valid()) hovered_event(*this, elem);
                    }
                    m_cursor = elem.get_cursor();
                } else {
                    if (released_event.valid()) released_event(*this, elem);
                }
            } else {
                if (released_event.valid()) released_event(*this, elem);
            }

            push_element(elem, siz, pos, render_index, texture_index, uv);
        }

        vec<element> child_elements = elem.get_objects();
        f32 x_offset = 0; f32 y_offset = 0;
        push_elements(child_elements, render_index, siz, pos, 
            elem.get_layout(), elem.get_padding(), 
            x_offset, y_offset, elem.get_alignment(),
            cursor_pt, pt_hwnd, left_mouse_state
        );
    }
}

inline auto ulx::context::update_scene_objects() -> void {
    POINT cursor_pt; GetCursorPos(&cursor_pt);
    HWND pt_hwnd = WindowFromPoint(cursor_pt);
    ScreenToClient(m_mainwin, &cursor_pt);
    int left_mouse_state = GetAsyncKeyState(VK_LBUTTON);

    u32 render_index = 0;
    m_vertices.clear(); m_indices.clear();
    m_scene = m_rendercallback(*this);
    vec<element>& objects = m_scene.get_objects();
    m_background_color = m_scene.get_background_color();

    m_cursor = resetcur;
    f32 x_offset = 0, y_offset = 0;
    push_elements(objects, render_index,
        {wfn::logical(m_mainwin_width, m_mainwin_dpi), wfn::logical(m_mainwin_height, m_mainwin_dpi)},
        {0, 0}, m_scene.get_layout(), m_scene.get_padding(), x_offset, y_offset, m_scene.get_alignment(),
        cursor_pt, pt_hwnd, left_mouse_state
    );

    if (m_texture_pushtag) push_texture_images();
}

inline auto ulx::context::set_tick(const tickcallback& callback, u64 delay) -> void { 
    m_tickcallback = callback; 
    m_delay = delay;
}
inline auto ulx::context::win_dpi() const -> UINT { return m_mainwin_dpi; }
inline auto ulx::context::win_cursor() const -> point {
    POINT pt; GetCursorPos(&pt);
    ScreenToClient(m_mainwin, &pt);
    return {wfn::logical(pt.x), wfn::logical(pt.y)};
}
inline auto ulx::context::win_max() -> void { m_maxtag = true; }
inline auto ulx::context::win_min() -> void { m_mintag = true; }
inline auto ulx::context::win_restore() -> void { m_restoretag = true; }
inline auto ulx::context::win_maxed() const -> bool { return IsZoomed(m_mainwin); }
inline auto ulx::context::win_mined() const -> bool { return IsIconic(m_mainwin); }

inline auto ulx::context::snapwin_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT {
    switch (message) {
        case WM_NCHITTEST:
            if (!IsZoomed(GetParent(hwnd))) {
                return HTMAXBUTTON;
            }
        case WM_NCLBUTTONDOWN:
            return HTTRANSPARENT;
    }
    return DefWindowProcA(hwnd, message, wparam, lparam);
}

inline auto ulx::context::win32_process(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT {
    switch (message) {
        case WM_NCCREATE: {
            auto* create_structure = reinterpret_cast<CREATESTRUCTW*>(lparam);
            auto* ctx = static_cast<context*>(create_structure->lpCreateParams);

            WNDCLASSA snap_trigger_window_class = {};
            snap_trigger_window_class.lpfnWndProc = snapwin_process;
            snap_trigger_window_class.hInstance = wfn::hinstance;
            snap_trigger_window_class.lpszClassName = "SnapTriggerWindow";
            ulx::log::expect(SUCCEEDED(RegisterClassA(&snap_trigger_window_class)), "context.hpp: ulx::context::win32_process(HWND, UINT, WPARAM, LPARAM): RegisterClassA() < 0, error code: {}", GetLastError());

            ctx->m_snapwin = CreateWindowExA(WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
                "SnapTriggerWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL,
                wfn::hinstance, NULL);
            ulx::log::expect(ctx->m_snapwin, "context.hpp: ulx::context::win32_process(HWND, UINT, WPARAM, LPARAM): CreateWindowExA() == NULL, error code: {}", GetLastError());

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
            if (!ctx->m_mainwin_resizable) return HTCLIENT;

            POINT cursor_position; GetCursorPos(&cursor_position);
            RECT window_rect; GetWindowRect(hwnd, &window_rect);
            int frame = GetSystemMetricsForDpi(SM_CYFRAME, ctx->win_dpi());
            POINT cursor_position_global = cursor_position;
            ScreenToClient(hwnd, &cursor_position_global);

            // move max button
            MoveWindow(ctx->m_snapwin,
                ctx->m_mainwin_width - ctx->m_vclosebtn_width - ctx->m_vmaxbtn_width, 0,
                ctx->m_vmaxbtn_width, ctx->m_vtitlebar_height, TRUE);

            // top edge
            if (cursor_position.y >= window_rect.top - frame && cursor_position.y < window_rect.top + frame)
                return HTTOP;

            // topbar
            if (!ctx->m_mainwin_titled && cursor_position_global.y >= 0 && cursor_position_global.y < ctx->m_vtitlebar_height) {
                if (cursor_position_global.x >= 0 &&
                    cursor_position_global.x < ctx->m_mainwin_width
                        - ctx->m_vclosebtn_width
                        - ctx->m_vminbtn_width
                        - ctx->m_vmaxbtn_width)
                    return HTCAPTION;
            }

            // another edges and corner cases
            return DefWindowProcW(hwnd, message, wparam, lparam);
        } case WM_NCCALCSIZE: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx || ctx->m_mainwin_titled) return DefWindowProcW(hwnd, message, wparam, lparam);

            // move titlebar out of client
            auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
            params->rgrc[0].top -= ctx->m_native_titleh;

            return DefWindowProcW(hwnd, WM_NCCALCSIZE, wparam, lparam);
        } case WM_PAINT: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            ctx->render_frame();

            ValidateRect(hwnd, nullptr);
            return 0;
        } case WM_GETMINMAXINFO: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            ulx::size min_size = ctx->m_mainwin_minsize;
            ulx::size max_size = ctx->m_mainwin_maxsize;

            MINMAXINFO* minmax_info = reinterpret_cast<MINMAXINFO*>(lparam);
            minmax_info->ptMinTrackSize.x = min_size.width();
            minmax_info->ptMinTrackSize.y = min_size.height();
            minmax_info->ptMaxTrackSize.x = max_size.width();
            minmax_info->ptMaxTrackSize.y = max_size.height();

            return 0;
        } case WM_SIZE: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);
            if (ctx->m_first) {
                ctx->m_first = false;
                return DefWindowProcW(hwnd, message, wparam, lparam);
            }

            ctx->m_dirty = true;
            ctx->render_frame();

            return 1;
        } case WM_SETCURSOR: {
            auto* ctx = reinterpret_cast<context*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!ctx) return DefWindowProcW(hwnd, message, wparam, lparam);

            switch (ctx->m_cursor) {
                case resetcur: return DefWindowProcW(hwnd, message, wparam, lparam);
                case arrowcur: SetCursor(LoadCursor(nullptr, IDC_ARROW)); return true;
                case handcur: SetCursor(LoadCursor(nullptr, IDC_HAND)); return true;
                case ibeamcur: SetCursor(LoadCursor(nullptr, IDC_IBEAM)); return true;
                case crosscur: SetCursor(LoadCursor(nullptr, IDC_CROSS)); return true;
                case waitcur: SetCursor(LoadCursor(nullptr, IDC_WAIT)); return true;
                case vhsizcur: SetCursor(LoadCursor(nullptr, IDC_SIZEWE)); return true;
                case vsizcur: SetCursor(LoadCursor(nullptr, IDC_SIZENS)); return true;
                case hsizcur: SetCursor(LoadCursor(nullptr, IDC_SIZEWE)); return true;
                case nocur: SetCursor(LoadCursor(nullptr, IDC_NO)); return true;
                case helpcur: SetCursor(LoadCursor(nullptr, IDC_HELP)); return true;
                case uparrowcur: SetCursor(LoadCursor(nullptr, IDC_UPARROW)); return true;
                case appstartingcur: SetCursor(LoadCursor(nullptr, IDC_APPSTARTING)); return true;
                case ldiagcur: SetCursor(LoadCursor(nullptr, IDC_SIZENWSE)); return true;
                case rdiagcur: SetCursor(LoadCursor(nullptr, IDC_SIZENESW)); return true;
                case sizallcur: SetCursor(LoadCursor(nullptr, IDC_SIZEALL)); return true;
            }

            return DefWindowProcW(hwnd, message, wparam, lparam);
        }
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

inline auto ulx::context::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] auto* user_data) -> VkBool32 {
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            log::warn("context.hpp: ulx::context::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, auto*): switch (message_severity): case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {}", callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            log::err("context.hpp: ulx::context::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, auto*): switch (message_severity): case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {}", callback_data->pMessage);
        default: break;
    }

    return VK_FALSE;
}

inline auto ulx::context::create_window(const wininfo& window_info, const appinfo& app_info) -> void {
    str window_class_name = window_info.get_window_class_name();
    str window_title = window_info.get_window_title();
    wstr window_wide_class_name = wstr(window_class_name.begin(), window_class_name.end());
    wstr window_wide_title = wstr(window_title.begin(), window_title.end());
    path icon_file_path = app_info.get_icon_file_path();
    pathstr icon_file_path_str = icon_file_path.str();
    wstr window_wide_icon = wstr(icon_file_path_str.begin(), icon_file_path_str.end());

    ulx::size window_size = window_info.get_initial_window_size();
    point window_pos = window_info.get_initial_window_pos();

    m_mainwin_minsize = window_info.get_min_window_size();
    m_mainwin_maxsize = window_info.get_max_window_size();
    if (window_size.width() < m_mainwin_minsize.width()) window_size = {m_mainwin_minsize.width(), window_size.height()};
    if (window_size.height() < m_mainwin_minsize.height()) window_size = {window_size.width(), m_mainwin_minsize.height()};
    if (window_size.width() > m_mainwin_maxsize.width()) window_size = {m_mainwin_maxsize.width(), window_size.height()};
    if (window_size.height() > m_mainwin_maxsize.height()) window_size = {window_size.width(), m_mainwin_maxsize.height()};
    m_mainwin_minsize = {wfn::physical(m_mainwin_minsize.width()), wfn::physical(m_mainwin_minsize.height())};
    m_mainwin_maxsize = {wfn::physical(m_mainwin_maxsize.width()), wfn::physical(m_mainwin_maxsize.height())};


    u32 physical_window_width = wfn::physical(window_size.width());
    u32 physical_window_height = wfn::physical(window_size.height());
    i32 physical_window_x = wfn::physical(window_pos.x());
    i32 physical_window_y = wfn::physical(window_pos.y());

    DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    if (window_info.get_window_attrs() & titledattr) {
        m_mainwin_titled = true;
    } else if (!(window_info.get_window_attrs() & borderedattr)) {
        window_style &= ~WS_BORDER;
        m_mainwin_resizable = false;
    }

    if (window_info.get_window_attrs() & resizableattr) {
        m_mainwin_resizable = true;
    } else {
        window_style &= ~WS_THICKFRAME;
        window_style &= ~WS_MAXIMIZEBOX;
    }

    WNDCLASSEXW main_window_class{};
    main_window_class.cbSize = sizeof(WNDCLASSEXW);
    main_window_class.style = CS_HREDRAW | CS_VREDRAW;
    main_window_class.lpfnWndProc = win32_process;
    main_window_class.hInstance = wfn::hinstance;
    main_window_class.hCursor = LoadCursorW(NULL, wfn::make_int_resource(32512));
    main_window_class.lpszClassName = window_wide_class_name.c_str();
    if (icon_file_path.exists()) {
        main_window_class.hIcon = reinterpret_cast<HICON>(LoadImageW(
            NULL, window_wide_icon.c_str(),
            IMAGE_ICON, 0, 0,
            LR_LOADFROMFILE ));
        main_window_class.hIconSm = reinterpret_cast<HICON>(LoadImageW(
            NULL, window_wide_icon.c_str(),
            IMAGE_ICON, 0, 0,
            LR_LOADFROMFILE ));
    }
    log::expect(SUCCEEDED(RegisterClassExW(&main_window_class)), "context.hpp: ulx::context::create_window(const wininfo&, const appinfo&): RegisterClassExW() < 0, error code: {}", GetLastError());

    m_mainwin = CreateWindowExW(
        0, window_wide_class_name.c_str(), window_wide_title.c_str(),
        window_style, physical_window_x, physical_window_y, physical_window_width, physical_window_height,
        NULL, NULL, wfn::hinstance, this);
    log::expect(m_mainwin, "context.hpp: ulx::context::create_window(): CreateWindowExW() == NULL, error code: {}", GetLastError());

    m_mainwin_dpi = GetDpiForWindow(m_mainwin);
    ulx::window_dpi = m_mainwin_dpi;
    __uii::fgg::window_dpi = m_mainwin_dpi;
    SetWindowLongPtrW(m_mainwin, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // update window
    int caption_height = GetSystemMetricsForDpi(SM_CYCAPTION, m_mainwin_dpi);
    int border_padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, m_mainwin_dpi);
    int border_size = GetSystemMetricsForDpi(SM_CXBORDER, m_mainwin_dpi);
    m_native_titleh = caption_height + border_padding + border_size*2 + 1;
    SetWindowPos(m_mainwin, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    RECT rect; GetClientRect(m_mainwin, &rect);
    m_mainwin_width = rect.right - rect.left;
    m_mainwin_height = rect.bottom - rect.top;
    m_vtitlebar_height = wfn::physical(window_info.get_titlebar_height(), m_mainwin_dpi);
    m_vclosebtn_width = wfn::physical(window_info.get_close_button_width(), m_mainwin_dpi);
    m_vminbtn_width = wfn::physical(window_info.get_min_button_width(), m_mainwin_dpi);
    m_vmaxbtn_width = wfn::physical(window_info.get_max_button_width(), m_mainwin_dpi);
}

inline constexpr ulx::context::context(const appinfo& application_info, const wininfo& window_info, const renderinfo& render_info) {
    m_cache_path = application_info.get_cache_dir();
    read_cache();
    create_window(window_info, application_info);
    create_vulkan_objects(application_info, render_info);
}

inline ulx::context::~context() {
    destroy_vulkan_objects();
    save_texture_caches();
}

inline auto ulx::context::exec() -> int {
    MSG msg{}; while (true) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return msg.wParam;
            DispatchMessageW(&msg);
        } else render_frame();
    }

    return 0;
}

inline auto ulx::context::win_set_minsize(const ulx::size& size) -> void { m_mainwin_minsize = size; }
inline auto ulx::context::win_set_maxsize(const ulx::size& size) -> void { m_mainwin_maxsize = size; }
inline auto ulx::context::win_show() const -> void { ShowWindow(m_mainwin, SW_SHOW); UpdateWindow(m_mainwin); }
inline auto ulx::context::win_hide() const -> void { ShowWindow(m_mainwin, SW_HIDE); UpdateWindow(m_mainwin); }
inline auto ulx::context::exit() const -> void { PostMessageW(m_mainwin, WM_CLOSE, 0, 0); }
inline auto ulx::context::win_size() const -> ulx::size { return {wfn::logical(m_mainwin_width), wfn::logical(m_mainwin_height)}; }
inline auto ulx::context::win_width() const -> f32 { return wfn::logical(m_mainwin_width); }
inline auto ulx::context::win_height() const -> f32 { return wfn::logical(m_mainwin_height); }
inline auto ulx::context::win_pos() const -> point {
    RECT window_rect{}; GetWindowRect(m_mainwin, &window_rect);
    return {wfn::logical(window_rect.left), wfn::logical(window_rect.top)};
}
inline auto ulx::context::win_x() const -> f32 { return win_pos().x(); }
inline auto ulx::context::win_y() const -> f32 { return win_pos().y(); }


inline auto ulx::context::win_move(const point& pos) -> void {
    i32 physical_x = wfn::physical(pos.x());
    i32 physical_y = wfn::physical(pos.y());
    MoveWindow(m_mainwin, physical_x, physical_y, m_mainwin_width, m_mainwin_height, TRUE);
}

inline auto ulx::context::win_resize(const ulx::size& size) -> void {
    u32 physical_width = wfn::physical(size.width());
    u32 physical_height = wfn::physical(size.height());
    MoveWindow(m_mainwin, win_x(), win_y(), physical_width, physical_height, TRUE);
    m_mainwin_width = physical_width;
    m_mainwin_height = physical_height;
}

inline auto ulx::context::create_vulkan_instance() -> void {
    VkApplicationInfo vulkan_application_info{};
    vulkan_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vulkan_application_info.pApplicationName = m_appid.cstr();
    vulkan_application_info.applicationVersion = m_appversion;
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
        log::expect(check_validation_layer_support(), "context.hpp: ulx::context::create_vulkan_instance(): check_validation_layer_support() == false");

        vulkan_instance_create_info.enabledLayerCount++;
        vulkan_instance_create_info.enabledExtensionCount++;

        debug_utils_messenger_create_info = {};
        debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_utils_messenger_create_info.pfnUserCallback = debug_callback;
        vulkan_instance_create_info.pNext = &debug_utils_messenger_create_info;
    #endif

    ulx::log::vk(vkCreateInstance(&vulkan_instance_create_info, VK_NULL_HANDLE, &m_vulkan_instance), "context.hpp: ulx::context::create_vulkan_instance(): vkCreateInstance() != VK_SUCCESS");
}

inline auto ulx::context::check_validation_layer_support() -> bool {
    u32 layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, VK_NULL_HANDLE);
    vec<VkLayerProperties> available_layers = vec<VkLayerProperties>(layer_count);
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

        auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_vulkan_instance, "vkCreateDebugUtilsMessengerEXT"));
        log::expect(vkCreateDebugUtilsMessengerEXT, "context.hpp: ulx::context::create_debug_messenger(): vkGetInstanceProcAddr() == VK_NULL_HANDLE");
        log::vk(vkCreateDebugUtilsMessengerEXT(m_vulkan_instance, &debug_utils_messenger_create_info, VK_NULL_HANDLE, &m_debug_messenger),
            "context.hpp: ulx::context::create_debug_messenger(): vkCreateDebugUtilsMessengerEXT() != VK_SUCCESS");
    #endif
}

inline auto ulx::context::destroy_debug_utils_messenger() -> void {
    if (m_debug_messenger == VK_NULL_HANDLE) return;
    auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_vulkan_instance, "vkDestroyDebugUtilsMessengerEXT"));
    log::expect(vkDestroyDebugUtilsMessengerEXT, "context.hpp: ulx::context::destroy_debug_utils_messenger(): vkGetInstanceProcAddr() == VK_NULL_HANDLE");
    vkDestroyDebugUtilsMessengerEXT(m_vulkan_instance, m_debug_messenger, VK_NULL_HANDLE);
}

inline auto ulx::context::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) -> void {
    debug_utils_messenger_create_info = {};
    debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_utils_messenger_create_info.pfnUserCallback = debug_callback;
}

inline auto ulx::context::select_physical_device() -> void {
    u32 physical_device_count = 0;
    vkEnumeratePhysicalDevices(m_vulkan_instance, &physical_device_count, VK_NULL_HANDLE);
    log::expect(physical_device_count > 0, "context.hpp: ulx::context::select_physical_device(): physical_device_count == 0");

    vec<VkPhysicalDevice> physical_devices = vec<VkPhysicalDevice>(physical_device_count);
    vkEnumeratePhysicalDevices(m_vulkan_instance, &physical_device_count, physical_devices.data());

    VkPhysicalDevice alternative_device = VK_NULL_HANDLE;
    __uii::vkclses::physical_device_info alternative_device_infos;
    u32 last_discrete_gpu_score = 0;
    u32 last_alternative_device_score = 0;

    for (auto& device : physical_devices) {
        __uii::vkclses::physical_device_info device_infos = __uii::vkclses::physical_device_info(device, m_window_surface);
        if (!device_infos.is_suitable()) continue;
        u32 score = __uii::vkalg::get_device_score(device_infos);
        if (device_infos.match_gpu_type(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && score > last_discrete_gpu_score) {
            last_discrete_gpu_score = score;
            m_physical_device = device;
            m_physical_device_infos = device_infos;
        } else if (device_infos.match_gpu_type(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && score > last_alternative_device_score) {
            last_alternative_device_score = score;
            alternative_device = device;
            alternative_device_infos = device_infos;
        }
    }

    if (m_physical_device != VK_NULL_HANDLE) return;
    log::expect(alternative_device != VK_NULL_HANDLE, "context.hpp: ulx::context::select_physical_device(): alternative_device == VK_NULL_HANDLE");

    m_physical_device = alternative_device;
    m_physical_device_infos = alternative_device_infos;
}

inline auto ulx::context::create_logical_device() -> void {
    vec<VkDeviceQueueCreateInfo> device_queue_create_infos;
    set<u32> unique_queue_family_indices = m_physical_device_infos.get_unique_queue_family_indices();
    f32 queue_priorities = 1.0f;
    for (u32 index : unique_queue_family_indices) {
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
    device_create_info.queueCreateInfoCount = static_cast<u32>(device_queue_create_infos.size());
    device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
    device_create_info.pEnabledFeatures = &physical_device_features;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = __uii::vkreqs::enabled_extensions;
    device_create_info.pNext = &features12;
    log::vk(vkCreateDevice(m_physical_device, &device_create_info, VK_NULL_HANDLE, &m_logical_device),
        "context.hpp: ulx::context::create_logical_device(): vkCreateDevice() != VK_SUCCESS");

    m_device_queues = __uii::vkclses::device_queues(m_logical_device, m_physical_device_infos);
}

inline auto ulx::context::create_window_surface() -> void {
    VkWin32SurfaceCreateInfoKHR win32_surface_create_info{};
    win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32_surface_create_info.hwnd = m_mainwin;
    win32_surface_create_info.hinstance = ulx::wfn::hinstance;
    log::vk(vkCreateWin32SurfaceKHR(m_vulkan_instance, &win32_surface_create_info, VK_NULL_HANDLE, &m_window_surface),
        "context.hpp: ulx::context::create_window_surface(): vkCreateWin32SurfaceKHR() != VK_SUCCESS");
}

inline auto ulx::context::create_swapchain(VkSwapchainKHR old_swapchain) -> void {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_window_surface, &m_physical_device_infos.swapchain_details.surface_capabilities);
    __uii::vkclses::swapchain_support_details& swapchain_support_details = m_physical_device_infos.swapchain_details;
    __uii::vkclses::queue_family_indices& optional_queue_family_indices = m_physical_device_infos.queue_family_indices;
    VkSurfaceFormatKHR surface_format = swapchain_support_details.select_surface_format();
    VkExtent2D extent = swapchain_support_details.select_extent(m_mainwin);
    u32 queue_family_indices[] = { optional_queue_family_indices.graphics_queue_family_index.value(), optional_queue_family_indices.present_queue_family_index.value() };
    u32 min_image_count = swapchain_support_details.surface_capabilities.minImageCount + 1;
    if (swapchain_support_details.surface_capabilities.maxImageCount > 0 && min_image_count > swapchain_support_details.surface_capabilities.maxImageCount)
        min_image_count = swapchain_support_details.surface_capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = m_window_surface;
    swapchain_create_info.minImageCount = min_image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = swapchain_support_details.surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain_support_details.select_present_mode(m_presentmode);
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.oldSwapchain = old_swapchain;
    if (optional_queue_family_indices.graphics_queue_family_index != optional_queue_family_indices.present_queue_family_index) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices; };
    log::vk(vkCreateSwapchainKHR(m_logical_device, &swapchain_create_info, VK_NULL_HANDLE, &m_swapchain),
        "context.hpp: ulx::context::create_swapchain(VkSwapchainKHR): vkCreateSwapchainKHR() != VK_SUCCESS");

    m_swapchain_image_format = surface_format.format;
    m_swapchain_extent = extent;
    m_mainwin_width = extent.width;
    m_mainwin_height = extent.height;
    get_swapchain_images();
}

inline auto ulx::context::create_render_finished_semaphores() -> void {
    m_render_finished_semaphores.resize(m_max_frames);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (u32 index = 0; index < m_max_frames; index++) {
        log::vk(vkCreateSemaphore(m_logical_device, &semaphore_info, nullptr, &m_render_finished_semaphores[index]),
            "context.hpp: ulx::context::create_render_finished_semaphores_per_image(): vkCreateSemaphore() != VK_SUCCESS");
    }
}

inline auto ulx::context::get_swapchain_images() -> void {
    u32 image_count = 0;
    vkGetSwapchainImagesKHR(m_logical_device, m_swapchain, &image_count, VK_NULL_HANDLE);
    m_swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_logical_device, m_swapchain, &image_count, m_swapchain_images.data());

    m_max_frames = image_count;
}

inline auto ulx::context::create_swapchain_image_views() -> void {
    m_swapchain_image_views.resize(m_swapchain_images.size());

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = m_swapchain_image_format;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    for (u64 index = 0; index < m_swapchain_images.size(); index++) {
        image_view_create_info.image = m_swapchain_images[index];
        log::vk(vkCreateImageView(m_logical_device, &image_view_create_info, nullptr, &m_swapchain_image_views[index]), "failed to create swapchain image views");
    }
}

inline auto ulx::context::create_shader_module(unsigned char* code, unsigned int length) -> VkShaderModule {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = length;
    shader_module_create_info.pCode = reinterpret_cast<ulx::u32*>(code);

    VkShaderModule shader_module;
    log::vk(vkCreateShaderModule(m_logical_device, &shader_module_create_info, VK_NULL_HANDLE, &shader_module),
        "context.hpp: ulx::context::create_shader_module(): vkCreateShaderModule() != VK_SUCCESS");


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

    VkVertexInputBindingDescription vertex_input_binding_description = __uii::vsdces::vertex::get_binding_description();
    vec<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions = __uii::vsdces::vertex::get_attribute_descriptions();

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
    viewport.width = static_cast<f32>(m_swapchain_extent.width);
    viewport.height = static_cast<f32>(m_swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain_extent;

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
    push_constant_range.size = sizeof(__uii::vsdces::pushconstant);

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &m_descriptor_set_layout;
    log::vk(vkCreatePipelineLayout(m_logical_device, &pipeline_layout_create_info, VK_NULL_HANDLE, &m_pipeline_layout),
        "context.hpp: ulx::context::create_graphics_pipeline(): vkCreatePipelineLayout() != VK_SUCCESS");


    path cache_file_path = m_cache_path.with("vkcache.bin");
    __uii::vkalg::ensure_cache_exists(cache_file_path);
    vec<char> cache_data = __uii::vkalg::read_cache(cache_file_path);
    VkPipelineCacheCreateInfo pipeline_cache_create_info{};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipeline_cache_create_info.initialDataSize = cache_data.size();
    pipeline_cache_create_info.pInitialData = cache_data.data();
    VkPipelineCache pipeline_cache;
    log::vk(vkCreatePipelineCache(m_logical_device, &pipeline_cache_create_info, nullptr, &pipeline_cache),
        "context.hpp: ulx::context::create_graphics_pipeline(): vkCreatePipelineCache() != VK_SUCCESS");


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
    graphics_pipeline_create_info.layout = m_pipeline_layout;
    graphics_pipeline_create_info.renderPass = m_render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;
    log::vk(vkCreateGraphicsPipelines(m_logical_device, pipeline_cache, 1, &graphics_pipeline_create_info, VK_NULL_HANDLE, &m_graphics_pipeline),
        "context.hpp: ulx::context::create_graphics_pipeline(): vkCreateGraphicsPipelines() != VK_SUCCESS");


    ulx::u64 cache_data_size;
    vec<char> new_cache_data;
    vkGetPipelineCacheData(m_logical_device, pipeline_cache, &cache_data_size, nullptr);
    new_cache_data.resize(cache_data_size);
    vkGetPipelineCacheData(m_logical_device, pipeline_cache, &cache_data_size, new_cache_data.data());
    __uii::vkalg::write_cache(cache_file_path.str(), new_cache_data);
    vkDestroyPipelineCache(m_logical_device, pipeline_cache, nullptr);
    vkDestroyShaderModule(m_logical_device, fragment_shader_module, VK_NULL_HANDLE);
    vkDestroyShaderModule(m_logical_device, vertex_shader_module, VK_NULL_HANDLE);
}

inline auto ulx::context::create_render_pass() -> void {
    VkAttachmentDescription color_attachment_description{};
    color_attachment_description.format = m_swapchain_image_format;
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
    log::vk(vkCreateRenderPass(m_logical_device, &render_pass_create_info, VK_NULL_HANDLE, &m_render_pass),
        "context.hpp: ulx::context::create_render_pass(): vkCreateRenderPass() != VK_SUCCESS");
}

inline auto ulx::context::create_swapchain_frame_buffers() -> void {
    m_swapchain_frame_buffers.resize(m_swapchain_images.size());

    VkFramebufferCreateInfo frame_buffer_create_info{};
    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.renderPass = m_render_pass;
    frame_buffer_create_info.attachmentCount = 1;
    frame_buffer_create_info.width = m_swapchain_extent.width;
    frame_buffer_create_info.height = m_swapchain_extent.height;
    frame_buffer_create_info.layers = 1;
    for (u32 index = 0; index < m_max_frames; index++) {
        frame_buffer_create_info.pAttachments = &m_swapchain_image_views[index];
        log::vk(vkCreateFramebuffer(m_logical_device, &frame_buffer_create_info, VK_NULL_HANDLE, &m_swapchain_frame_buffers[index]),
            "context.hpp: ulx::context::create_swapchain_frame_buffers(): vkCreateFramebuffer() != VK_SUCCESS");
    }
}

inline auto ulx::context::create_command_pool() -> void {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = m_physical_device_infos.queue_family_indices.graphics_queue_family_index.value();
    log::vk(vkCreateCommandPool(m_logical_device, &command_pool_create_info, VK_NULL_HANDLE, &m_command_pool),
        "context.hpp: ulx::context::create_command_pool(): vkCreateCommandPool() != VK_SUCCESS");
}

inline auto ulx::context::create_command_buffers() -> void {
    m_command_buffers.resize(m_max_frames);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = m_command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = static_cast<ulx::u32>(m_command_buffers.size());
    log::vk(vkAllocateCommandBuffers(m_logical_device, &command_buffer_allocate_info, m_command_buffers.data()),
        "context.hpp: ulx::context::create_command_buffers(): vkAllocateCommandBuffers() != VK_SUCCESS");
}

inline auto ulx::context::create_sync_objects() -> void {
    m_image_available_semaphores.resize(m_max_frames);
    m_in_flight_fences.resize(m_max_frames);

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u64 index = 0; index < m_max_frames; index++) {
        log::vk(vkCreateSemaphore(m_logical_device, &semaphore_create_info, VK_NULL_HANDLE, &m_image_available_semaphores[index]),
            "context.hpp: ulx::context::create_sync_objects(): vkCreateSemaphore() != VK_SUCCESS");
        log::vk(vkCreateFence(m_logical_device, &fence_create_info, VK_NULL_HANDLE, &m_in_flight_fences[index]),
            "context.hpp: ulx::context::create_sync_objects(): vkCreateFence() != VK_SUCCESS");
    }
}

inline auto ulx::context::destroy_swapchain() -> void {
    for (const auto& frame_buffer : m_swapchain_frame_buffers)
        vkDestroyFramebuffer(m_logical_device, frame_buffer, VK_NULL_HANDLE);
    for (const auto& image_view : m_swapchain_image_views)
        vkDestroyImageView(m_logical_device, image_view, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(m_logical_device, m_swapchain, VK_NULL_HANDLE);
}

inline auto ulx::context::recreate_swapchain() -> void {
    RECT window_rect;
    GetClientRect(m_mainwin, &window_rect);
    u32 width = window_rect.right - window_rect.left;
    u32 height = window_rect.bottom - window_rect.top;

    MSG msg;
    while (width == 0 || height == 0) {
        MsgWaitForMultipleObjects(0, nullptr, FALSE, INFINITE, QS_ALLINPUT);

        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        GetClientRect(m_mainwin, &window_rect);
        width = window_rect.right - window_rect.left;
        height = window_rect.bottom - window_rect.top;
    }

    VkSwapchainKHR old_swapchain = m_swapchain;
    vec<VkFramebuffer> old_framebuffers = std::move(m_swapchain_frame_buffers);
    vec<VkImageView> old_image_views = std::move(m_swapchain_image_views);
    vec<VkSemaphore> old_render_finished_semaphores = std::move(m_render_finished_semaphores);

    create_swapchain(old_swapchain);
    create_swapchain_image_views();
    create_swapchain_frame_buffers();
    create_render_finished_semaphores();

    m_pending_framebuffers.push_back(std::move(old_framebuffers));
    m_pending_image_views.push_back(std::move(old_image_views));
    m_pending_swapchains.push_back(old_swapchain);
    m_pending_semaphores.push_back(std::move(old_render_finished_semaphores));

    m_dirty = false;
}

inline auto ulx::context::create_texture_buffer() -> void {
    VkDeviceSize texture_size = create_texture_images() * 1.5;

    m_texture_size = texture_size;
    m_texture_buffer = __uii::vkclses::buffer(
        m_physical_device, m_logical_device, m_texture_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
}

inline auto ulx::context::create_vertex_buffer() -> void {
    VkDeviceSize size = m_vertices.size() * sizeof(__uii::vsdces::vertex);

    if (m_vertex_size < size) {
        if (m_vertex_buffer.valid()) m_vertex_buffer.release(m_logical_device);
        m_vertex_size = size * 1.5;

        m_vertex_buffer = __uii::vkclses::buffer(
            m_physical_device, m_logical_device, m_vertex_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
    }
}

inline auto ulx::context::create_index_buffer() -> void {
    VkDeviceSize size = m_indices.size() * sizeof(ulx::u32);

    if (m_index_size < size) {
        if (m_index_buffer.valid()) m_index_buffer.release(m_logical_device);
        m_index_size = size * 1.5;

        m_index_buffer = __uii::vkclses::buffer(
            m_physical_device, m_logical_device, m_index_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
    }
}

inline auto ulx::context::update_object_buffers() -> void {
    create_vertex_buffer();
    create_index_buffer();

    m_vertex_buffer.map_memory(m_logical_device, m_vertices.data(), m_vertices.size() * sizeof(__uii::vsdces::vertex));
    m_index_buffer.map_memory(m_logical_device, m_indices.data(), m_indices.size() * sizeof(u32));
}

inline auto ulx::context::record_command_buffer(VkCommandBuffer command_buffer, u32 image_index) -> void {
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = VK_NULL_HANDLE;
    log::vk(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info),
        "context.hpp: ulx::context::record_command_buffer(VkCommandBuffer): vkBeginCommandBuffer() != VK_SUCCESS");

    VkClearValue clear_color = {{{m_background_color.red() / 255.0f, m_background_color.green() / 255.0f, m_background_color.blue() / 255.0f, m_background_color.alpha() / 255.0f}}};
    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = m_render_pass;
    render_pass_begin_info.framebuffer = m_swapchain_frame_buffers[image_index];
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = m_swapchain_extent;
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;
    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(m_swapchain_extent.width);
    viewport.height = static_cast<f32>(m_swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    constexpr VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_vertex_buffer.buf, offsets);
    vkCmdBindIndexBuffer(command_buffer, m_index_buffer.buf, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_descriptor_sets[m_current_frame], 0, nullptr);

    vkCmdPushConstants(command_buffer, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(__uii::vsdces::pushconstant), &m_pushconstant);
    vkCmdDrawIndexed(command_buffer, static_cast<u32>(m_indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    log::vk(vkEndCommandBuffer(command_buffer), "context.hpp: ulx::context::record_command_buffer(VkCommandBuffer): vkEndCommandBuffer() != VK_SUCCESS");
}

inline auto ulx::context::render_frame() -> void {
    if (!m_pending_framebuffers.empty()) {
        bool all_signaled = true;
        for (u32 index = 0; index < m_max_frames; index++) {
            VkResult status = vkGetFenceStatus(m_logical_device, m_in_flight_fences[index]);
            if (status != VK_SUCCESS) {
                all_signaled = false;
                break;
            }
        } if (all_signaled) {
            {
                std::lock_guard lock(m_pending_mutex);

                auto local_swapchains = std::move(m_pending_swapchains);

                for (u64 j = 0; j < m_pending_framebuffers.size(); j++) {
                    for (auto& fb : m_pending_framebuffers[j])
                        vkDestroyFramebuffer(m_logical_device, fb, nullptr);
                    for (auto& iv : m_pending_image_views[j])
                        vkDestroyImageView(m_logical_device, iv, nullptr);
                    for (auto& sem : m_pending_semaphores[j])
                        vkDestroySemaphore(m_logical_device, sem, nullptr);
                }

                m_pending_framebuffers.clear();
                m_pending_image_views.clear();
                m_pending_semaphores.clear();

                m_pending_tasks.push_back(std::async(std::launch::async, [this, chains = std::move(local_swapchains)]() {
                    for (auto sc : chains) {
                        vkDestroySwapchainKHR(m_logical_device, sc, nullptr);
                    }
                }));
            }
        }
    }

    if (m_maxtag) { ShowWindow(m_mainwin, SW_MAXIMIZE); m_maxtag = false; }
    if (m_mintag) { ShowWindow(m_mainwin, SW_MINIMIZE); m_mintag = false; }
    if (m_restoretag) { ShowWindow(m_mainwin, SW_RESTORE); m_restoretag = false; }

    vkWaitForFences(m_logical_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
    if (m_dirty) {
        recreate_swapchain();
        update_push_constant();
    }

    update_scene_objects();
    update_object_buffers();

    u32 image_index;
    VkResult acquire_result = vkAcquireNextImageKHR(m_logical_device, m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index);
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || acquire_result == VK_ERROR_SURFACE_LOST_KHR || acquire_result == VK_SUBOPTIMAL_KHR) {
        m_dirty = true;
        return;
    } else log::expect(acquire_result == VK_SUCCESS, "context.hpp: ulx::context::render(): acquire_result != VK_SUCCESS");

    vkResetFences(m_logical_device, 1, &m_in_flight_fences[m_current_frame]);
    vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
    record_command_buffer(m_command_buffers[m_current_frame], image_index);

    constexpr VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &m_image_available_semaphores[m_current_frame];
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffers[m_current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &m_render_finished_semaphores[image_index];
    log::vk(vkQueueSubmit(m_device_queues.graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]),
        "context.hpp: ulx::context::draw_frame(): vkQueueSubmit() != VK_SUCCESS");

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_render_finished_semaphores[image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain;
    present_info.pImageIndices = &image_index;
    vkQueuePresentKHR(m_device_queues.present_queue, &present_info);

    m_current_frame = (m_current_frame + 1) % m_max_frames;
    auto elapsed = ms_cast(steady::now() - m_lasttime).count();
    if (elapsed >= m_delay && m_tickcallback.valid()) {
        m_lasttime = steady::now();
        m_tickcallback(*this, elapsed);
    }
}

inline auto ulx::context::create_descriptor_set_layout() -> void {
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
    descriptor_set_layout_binding.binding = 0;
    descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_set_layout_binding.descriptorCount = 255;
    descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptor_set_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
    log::vk(vkCreateDescriptorSetLayout(m_logical_device, &descriptor_set_layout_create_info, nullptr, &m_descriptor_set_layout),
        "context.hpp: ulx::context::create_descriptor_set_layout(): vkCreateDescriptorSetLayout() != VK_SUCCESS");
}

inline auto ulx::context::create_descriptor_update_template() -> void {
    VkDescriptorUpdateTemplateEntry descriptor_update_template_entry{};
    descriptor_update_template_entry.dstBinding = 0;
    descriptor_update_template_entry.dstArrayElement = 0;
    descriptor_update_template_entry.descriptorCount = 255;
    descriptor_update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_update_template_entry.offset = 0;
    descriptor_update_template_entry.stride = sizeof(VkDescriptorImageInfo);

    VkDescriptorUpdateTemplateCreateInfo descriptor_update_template_create_info{};
    descriptor_update_template_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
    descriptor_update_template_create_info.descriptorUpdateEntryCount = 1;
    descriptor_update_template_create_info.pDescriptorUpdateEntries = &descriptor_update_template_entry;
    descriptor_update_template_create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    descriptor_update_template_create_info.descriptorSetLayout = m_descriptor_set_layout;
    log::vk(vkCreateDescriptorUpdateTemplate(m_logical_device, &descriptor_update_template_create_info, nullptr, &m_descriptor_update_template),
        "context.hpp: ulx::context::create_descriptor_update_template(): vkCreateDescriptorUpdateTemplate() != VK_SUCCESS");
}

inline auto ulx::context::create_descriptor_pools() -> void {
    m_descriptor_sets.resize(m_max_frames);

    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 255 * m_max_frames;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = m_max_frames;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    log::vk(vkCreateDescriptorPool(m_logical_device, &pool_info, nullptr, &m_descriptor_pool),
        "context.hpp: ulx::context::create_descriptor_pools(): vkCreateDescriptorPool() != VK_SUCCESS");
    for (u32 i = 0; i < m_max_frames; i++) {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = m_descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &m_descriptor_set_layout;
        log::vk(vkAllocateDescriptorSets(m_logical_device, &alloc_info, &m_descriptor_sets[i]),
            "context.hpp: ulx::context::create_descriptor_pools(): vkAllocateDescriptorSets() != VK_SUCCESS");
    }
    update_texture_descriptors();
}

inline auto ulx::context::init_descriptor_image_infos() -> void {
    m_descriptor_image_infos = vec<VkDescriptorImageInfo>(255);
    for (u64 index = 0; index < 255; index++) {
        m_descriptor_image_infos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_descriptor_image_infos[index].imageView = m_texture_images[0].image_view;
        m_descriptor_image_infos[index].sampler = m_texture_image_sampler;
    }
}

inline auto ulx::context::update_texture_descriptors() -> void {
    for (u64 index = 0; index < m_texture_images.size(); index++) {
        m_descriptor_image_infos[index].imageView = m_texture_images[index].image_view;
    }

    for (u64 index = 0; index < m_max_frames; index++) {
        vkUpdateDescriptorSetWithTemplate(m_logical_device, m_descriptor_sets[index], 
            m_descriptor_update_template, m_descriptor_image_infos.data());
    }
}

inline auto ulx::context::map_texture_images() -> void {
    vec<VkDeviceSize> offsets(m_texture_image_pixmaps.size());
    VkDeviceSize current_offset = 0;
    u32 memory_type_bits;

    for (u64 index = 0; index < m_texture_image_pixmaps.size(); ++index) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(m_logical_device, m_texture_images[index].image, &memory_requirements);
        current_offset = (current_offset + memory_requirements.alignment - 1) & ~(memory_requirements.alignment - 1);
        offsets[index] = current_offset;
        current_offset += memory_requirements.size;

        if (!index) memory_type_bits = memory_requirements.memoryTypeBits;
        else memory_type_bits &= memory_requirements.memoryTypeBits;
    }

    VkDeviceSize staging_total_size = 0;
    for (auto& pixmap : m_texture_image_pixmaps)
        staging_total_size += pixmap.pixel_size();

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = current_offset;
    memory_allocate_info.memoryTypeIndex = __uii::vkalg::find_memory_type(m_physical_device, memory_type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    log::vk(vkAllocateMemory(m_logical_device, &memory_allocate_info, nullptr, &m_texture_image_memory),
        "context.hpp: ulx::context::map_texture_images(): vkAllocateMemory() != VK_SUCCESS");

    for (u64 index = 0; index < m_texture_image_pixmaps.size(); index++)
        vkBindImageMemory(m_logical_device, m_texture_images[index].image, m_texture_image_memory, offsets[index]);

    void* mapped; VkDeviceSize staging_offset = 0;
    vkMapMemory(m_logical_device, m_texture_buffer.buffer_memory, 0, staging_total_size, 0, &mapped);
    VkCommandBuffer single_time_command_buffer = __uii::vkalg::start_single_time_command_buffer(m_logical_device, m_command_pool);
    for (u64 index = 0; index < m_texture_image_pixmaps.size(); index++) {
        VkImage image = m_texture_images[index].image;
        pixmap pixmap = m_texture_image_pixmaps[index];
        ulx::size size = pixmap.size();

        memcpy(reinterpret_cast<char*>(mapped) + staging_offset, pixmap.pixels().data(), pixmap.pixel_size());
        offsets[index] = staging_offset;
        staging_offset += pixmap.pixel_size();

        __uii::vkalg::transition_image_layout(m_logical_device, single_time_command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        __uii::vkalg::copy_buffer_to_image(m_logical_device, m_texture_buffer.buf, single_time_command_buffer, image, size.width(), size.height(), offsets[index]);
        __uii::vkalg::transition_image_layout(m_logical_device, single_time_command_buffer, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    __uii::vkalg::end_single_time_command_buffer(m_logical_device, m_device_queues.graphics_queue, m_command_pool, single_time_command_buffer);
    vkUnmapMemory(m_logical_device, m_texture_buffer.buffer_memory);
}

inline auto ulx::context::create_texture_image(ulx::u64 index) -> VkMemoryRequirements {
    VkImage& texture = m_texture_images[index].image;
    pixmap pixmap = m_texture_image_pixmaps[index];
    ulx::size size = pixmap.size();

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
    log::vk(vkCreateImage(m_logical_device, &image_create_info, VK_NULL_HANDLE, &texture),
        "context.hpp: ulx::context::create_texture_image(): vkCreateImage() != VK_SUCCESS");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(m_logical_device, texture, &memory_requirements);
    return memory_requirements;
}

inline auto ulx::context::create_texture_image_views() -> void {
    for (u64 index = 0; index < m_texture_images.size(); ++index)
        m_texture_images[index].image_view = __uii::vkalg::create_image_view(m_logical_device, m_texture_images[index].image, VK_FORMAT_R8G8B8A8_UNORM);
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
    sampler_create_info.maxAnisotropy = m_physical_device_infos.properties.limits.maxSamplerAnisotropy;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 0.0f;
    log::vk(vkCreateSampler(m_logical_device, &sampler_create_info, nullptr, &m_texture_image_sampler),
        "context.hpp: ulx::context::create_texture_image_sampler(): vkCreateSampler() != VK_SUCCESS");
}

inline auto ulx::context::push_texture_image(const ulx::pixmap& pixmap) -> void {
    m_texture_image_pixmaps.push_back(pixmap);
    m_texture_pushtag = true;
}

inline auto ulx::context::push_texture_images() -> void {
    m_texture_pushtag = false;

    auto old_textures = std::move(m_texture_images);
    auto old_memory = m_texture_image_memory;

    recreate_texture_buffer();

    map_texture_images();
    create_texture_image_views();
    vkDestroyDescriptorPool(m_logical_device, m_descriptor_pool, nullptr);
    create_descriptor_pools();

    for (auto& image : old_textures)
        image.release(m_logical_device);
    vkFreeMemory(m_logical_device, old_memory, nullptr);
}

inline auto ulx::context::recreate_texture_buffer() -> void {
    VkDeviceSize texture_total_size = 0;
    for (auto& pixmap : m_texture_image_pixmaps)
        texture_total_size += pixmap.pixel_size();

    if (m_texture_size >= texture_total_size) {
        create_texture_images();
        return;
    }

    m_texture_buffer.release(m_logical_device);
    create_texture_buffer();
}

inline auto ulx::context::create_texture_images() -> VkDeviceSize {
    VkDeviceSize size = 0; m_texture_images.resize(m_texture_image_pixmaps.size());
    for (ulx::u64 index = 0; index < m_texture_image_pixmaps.size(); index++) {
        VkMemoryRequirements memory_requirements = create_texture_image(index);
        size += memory_requirements.size;
    }

    return size;
}

inline auto ulx::context::save_texture_caches() -> void {
    vec<std::future<void>> pending_save_tasks;
    u32 i = 0;
    for (auto& pixmap : m_texture_image_pixmaps) {
        pending_save_tasks.push_back(std::async(std::launch::async, [&pixmap, i, this]() {
            pixmap.save(m_cache_path.with("texture_cache_" + pathstr::from(i) + ".png"));
        }));
        i++;
    }

    using namespace rapidjson;
    path cache_file_path = m_cache_path.with("texture_cache.json");

    Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    Value bitmap(kObjectType);
        for (auto& cache : m_bitmap_cache) {
            u32 idx = cache.second.get<0>();
            __uii::ats::uv uv = cache.second.get<1>();
            
            Value value(kArrayType);
                value.PushBack(idx, alloc);
                point pos = uv.get<0>();
                value.PushBack(pos.x(), alloc);
                value.PushBack(pos.y(), alloc);
                size sz = uv.get<1>();
                value.PushBack(sz.width(), alloc);
                value.PushBack(sz.height(), alloc);
    
            Value key(cache.first.str().cstr(), alloc);
            bitmap.AddMember(key, value, alloc);
        }
    doc.AddMember("bitmap", bitmap, alloc);

    Value vector(kArrayType);
        for (auto& cache : m_vector_cache) {
            Value key(kArrayType);
                path ps = cache.first.get<0>();
                size sz2 = cache.first.get<1>();
                Value vp(ps.str().cstr(), alloc);
                key.PushBack(vp, alloc);
                key.PushBack(sz2.width(), alloc);
                key.PushBack(sz2.height(), alloc);
            vector.PushBack(key, alloc);
            
            u32 idx = cache.second.get<0>();
            __uii::ats::uv uv = cache.second.get<1>();
            Value value(kArrayType);
                value.PushBack(idx, alloc);
                point pos = uv.get<0>();
                value.PushBack(pos.x(), alloc);
                value.PushBack(pos.y(), alloc);
                size sz = uv.get<1>();
                value.PushBack(sz.width(), alloc);
                value.PushBack(sz.height(), alloc);
            vector.PushBack(value, alloc);
        }
    doc.AddMember("vector", vector, alloc);

    Value font_(kArrayType);
        for (auto& cache : m_font_cache) {
            Value entry(kArrayType);
            Value char_val(cache.first.get<0>().cstr(), alloc);
            entry.PushBack(char_val, alloc);
    
            font f = cache.first.get<1>();
            Value font_name(f.get_name().cstr(), alloc);
            entry.PushBack(font_name, alloc);
            entry.PushBack(f.get_size(), alloc);
            entry.PushBack(cache.second.get<0>(), alloc);
            
            __uii::ats::uv uv = cache.second.get<1>();
            point pos = uv.get<0>();
            size sz = uv.get<1>();
            entry.PushBack(pos.x(), alloc);
            entry.PushBack(pos.y(), alloc);
            entry.PushBack(sz.width(), alloc);
            entry.PushBack(sz.height(), alloc);
            
            font_.PushBack(entry, alloc);
        }
    doc.AddMember("font", font_, alloc);


    Value atlas_cfg(kObjectType);
        atlas_cfg.AddMember("current_atlas_index", Value().SetUint(m_atlas_index), alloc);
        atlas_cfg.AddMember("current_atlas_x", Value().SetUint(m_atlas_x), alloc);
        atlas_cfg.AddMember("current_atlas_y", Value().SetUint(m_atlas_y), alloc);
        atlas_cfg.AddMember("current_atlas_max_height", Value().SetUint(m_atlas_height), alloc);
    doc.AddMember("atlas_cfg", atlas_cfg, alloc);
    
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    doc.Accept(writer);
    cache_file_path.write(str(buffer.GetString(), buffer.GetSize()));
    
    for (auto& task : pending_save_tasks) {
        task.get();
    }
}

inline auto ulx::context::read_cache() -> void {
    using namespace rapidjson;

    vec<std::future<pixmap>> pending_read_tasks;
    for (u32 i = 0;; i++) {
        path tex_path = m_cache_path.with("texture_cache_" + pathstr::from(i) + ".png");
        if (!tex_path.exists()) break;

        pending_read_tasks.push_back(
            std::async(std::launch::async, [path = std::move(tex_path)]() -> pixmap {
                return pixmap::from_bitmap(path);
            })
        );
    }

    path json_path = m_cache_path.with("texture_cache.json");
    if (json_path.exists()) {
        Document doc;
        str json_str = json_path.read_string();
        doc.Parse(json_str.c_str());

        if (doc.HasParseError() || !doc.IsObject()) {
            ulx::log::err("context.hpp: ulx::context::read_cache(): doc.HasParseError() == true || doc.IsObject() == false, maybe try clearing all the cache and then retry.");
        } else {
            if (doc.HasMember("bitmap") && doc["bitmap"].IsObject()) {
                auto bitmap_obj = doc["bitmap"].GetObject();
                for (auto& member : bitmap_obj) {
                    Value& arr = member.value;
                    u32 idx = arr[0].GetUint();
                    point pos(arr[1].GetFloat(), arr[2].GetFloat());
                    size sz(arr[3].GetFloat(), arr[4].GetFloat());
                    __uii::ats::uv uv = {pos, sz};
                    __uii::ats::idx cache_entry = {idx, uv};
                    m_bitmap_cache[path(member.name.GetString())] = cache_entry;
                }
            }

            if (doc.HasMember("vector") && doc["vector"].IsArray()) {
                auto vec_arr = doc["vector"].GetArray();
                for (size_t i = 0; i + 1 < vec_arr.Size(); i += 2) {
                    Value& key_arr = vec_arr[i];
                    Value& val_arr = vec_arr[i + 1];

                    path ps(key_arr[0].GetString());
                    size sz2(key_arr[1].GetFloat(), key_arr[2].GetFloat());
                    element::texture::vector key = {ps, sz2};

                    u32 idx = val_arr[0].GetUint();
                    point pos(val_arr[1].GetFloat(), val_arr[2].GetFloat());
                    size sz(val_arr[3].GetFloat(), val_arr[4].GetFloat());
                    __uii::ats::uv uv = {pos, sz};
                    __uii::ats::idx cache_entry = {idx, uv};
                    m_vector_cache[key] = cache_entry;
                }
            }

            if (doc.HasMember("font") && doc["font"].IsArray()) {
                auto font_arr = doc["font"].GetArray();
                
                for (size_t i = 0; i < font_arr.Size(); i++) {
                    Value& entry = font_arr[i];
                    
                    u8char ch(entry[0].GetString());
                    font f(entry[1].GetString(), entry[2].GetFloat());
                    
                    element::texture::text key = {ch, f};

                    u32 idx = entry[3].GetUint();
                    point pos(entry[4].GetFloat(), entry[5].GetFloat());
                    size sz(entry[6].GetFloat(), entry[7].GetFloat());

                    __uii::ats::uv uv = {pos, sz};
                    __uii::ats::idx cache_entry = {idx, uv};
                    m_font_cache[key] = cache_entry;
                }
            }

            if (doc.HasMember("atlas_cfg") && doc["atlas_cfg"].IsObject()) {
                auto& cfg = doc["atlas_cfg"];
                m_atlas_index = cfg["current_atlas_index"].GetUint();
                m_atlas_x = cfg["current_atlas_x"].GetUint();
                m_atlas_y = cfg["current_atlas_y"].GetUint();
                m_atlas_height = cfg["current_atlas_max_height"].GetUint();
            }
        }
    }

    if (pending_read_tasks.empty()) {
        m_texture_image_pixmaps = { __uii::vkalg::empty_pixmap(), pixmap(2048, 2048) };
        return;
    }
    for (auto& task : pending_read_tasks) {
        m_texture_image_pixmaps.push_back(task.get());
    }
}

inline auto ulx::context::destroy_vulkan_objects() -> void {
    vkDeviceWaitIdle(m_logical_device);

    for (auto& fn : m_pending_tasks) fn.get();
    for (u64 jndex = 0; jndex < m_pending_framebuffers.size(); jndex++) {
        for (auto& framebuffer : m_pending_framebuffers[jndex])
            vkDestroyFramebuffer(m_logical_device, framebuffer, nullptr);
        for (auto& imageview : m_pending_image_views[jndex])
            vkDestroyImageView(m_logical_device, imageview, nullptr);

        VkSwapchainKHR current_swapchain_handle = m_pending_swapchains[jndex];
        vkDestroySwapchainKHR(m_logical_device, current_swapchain_handle, nullptr);

        for (auto& semaphore : m_pending_semaphores[jndex])
            vkDestroySemaphore(m_logical_device, semaphore, nullptr);
    }

    vkDestroyDescriptorUpdateTemplate(m_logical_device, m_descriptor_update_template, VK_NULL_HANDLE);
    destroy_swapchain();
    vkDestroySampler(m_logical_device, m_texture_image_sampler, VK_NULL_HANDLE);
    for (auto& image : m_texture_images)
        image.release(m_logical_device);
    vkFreeMemory(m_logical_device, m_texture_image_memory, VK_NULL_HANDLE);
    vkDestroyDescriptorPool(m_logical_device, m_descriptor_pool, VK_NULL_HANDLE);
    
    vkDestroyDescriptorSetLayout(m_logical_device, m_descriptor_set_layout, VK_NULL_HANDLE);
    m_index_buffer.release(m_logical_device);
    m_vertex_buffer.release(m_logical_device);
    m_texture_buffer.release(m_logical_device);
    for (u32 i = 0; i < m_render_finished_semaphores.size(); i++)
        vkDestroySemaphore(m_logical_device, m_render_finished_semaphores[i], VK_NULL_HANDLE);
    for (u64 index = 0; index < m_max_frames; index++) {
        vkDestroySemaphore(m_logical_device, m_image_available_semaphores[index], VK_NULL_HANDLE);
        vkDestroyFence(m_logical_device, m_in_flight_fences[index], VK_NULL_HANDLE);
    }
    vkDestroyCommandPool(m_logical_device, m_command_pool, VK_NULL_HANDLE);
    vkDestroyPipeline(m_logical_device, m_graphics_pipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(m_logical_device, m_pipeline_layout, VK_NULL_HANDLE);
    vkDestroyRenderPass(m_logical_device, m_render_pass, VK_NULL_HANDLE);
    vkDestroyDevice(m_logical_device, VK_NULL_HANDLE);
    destroy_debug_utils_messenger();
    vkDestroySurfaceKHR(m_vulkan_instance, m_window_surface, VK_NULL_HANDLE);
    vkDestroyInstance(m_vulkan_instance, VK_NULL_HANDLE);
}

inline auto ulx::context::create_vulkan_objects(const appinfo& application_info, const renderinfo& render_info) -> void {
    m_rendercallback = render_info.get_render_callback();
    m_appid = application_info.get_application_id();
    m_appversion = application_info.get_application_version();
    m_presentmode = render_info.get_present_mode();

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
    create_texture_buffer();

    map_texture_images();

    create_texture_image_views();
    create_texture_image_sampler();
    init_descriptor_image_infos();
    create_descriptor_pools();
    create_render_finished_semaphores();

    update_scene_objects();
    update_push_constant();

    create_vertex_buffer();
    create_index_buffer();
    create_command_buffers();
    create_sync_objects();
    update_object_buffers();
}
