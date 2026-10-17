#pragma once

#include "size.hpp"
#include "point.hpp"
#include "winattr.hpp"
#include "types.hpp"
#include "requires.hpp"
#include <utility>

namespace ulx {
    class wininfo {
        private:
            u8 window_attrs = winattr::normalattr;
            str window_class_name;
            str window_title;
            size initial_window_size = {800, 600};
            point initial_window_pos = point(0, 0);
            size min_window_size = {800, 600};
            size max_window_size = {1200, 900};
            f32 titlebar_height = 30;
            
            f32 close_button_width = 45;
            f32 min_button_width = 45;
            f32 max_button_width = 45;
        
        public:
            inline constexpr wininfo() = default;

            template<typename T, typename CN> 
                requires expect<T, str> && expect<CN, str>
            inline constexpr wininfo(
                T&& window_title, 
                CN&& window_class_name = "UlixWindowClass"
            ):
                window_class_name(std::forward<CN>(window_class_name)),
                window_title(std::forward<T>(window_title))
            {}
        
            inline constexpr auto size(const ulx::size& rec) -> wininfo& { initial_window_size = rec; return *this; }
            inline constexpr auto pos(const point& rec) -> wininfo& { initial_window_pos = rec; return *this; }
            inline constexpr auto attr(u8 attrs) -> wininfo& { window_attrs |= attrs; return *this; }
            inline constexpr auto unattr(u8 attrs) -> wininfo& { window_attrs &= ~attrs; return *this; }
            inline constexpr auto min(const ulx::size& rec) -> wininfo& { min_window_size = rec; return *this; }
            inline constexpr auto max(const ulx::size& rec) -> wininfo& { max_window_size = rec; return *this; }
            inline constexpr auto vtitlebar(f32 height) -> wininfo& { titlebar_height = height; return *this; }
            inline constexpr auto vclose(f32 width) -> wininfo& { close_button_width = width; return *this; }
            inline constexpr auto vmin(f32 width) -> wininfo& { min_button_width = width; return *this; }
            inline constexpr auto vmax(f32 width) -> wininfo& { max_button_width = width; return *this; }
        
        public:
            inline constexpr auto get_window_attrs() const -> u8 { return window_attrs; }
            inline constexpr auto get_window_class_name() const -> const str& { return window_class_name; }
            inline constexpr auto get_window_title() const -> const str& { return window_title; }
            inline constexpr auto get_initial_window_size() const -> const ulx::size& { return initial_window_size; }
            inline constexpr auto get_initial_window_pos() const -> const point& { return initial_window_pos; }
            inline constexpr auto get_min_window_size() const -> const ulx::size& { return min_window_size; }
            inline constexpr auto get_max_window_size() const -> const ulx::size& { return max_window_size; }
            inline constexpr auto get_titlebar_height() const -> f32 { return titlebar_height; }
            inline constexpr auto get_close_button_width() const -> f32 { return close_button_width; }
            inline constexpr auto get_min_button_width() const -> f32 { return min_button_width; }
            inline constexpr auto get_max_button_width() const -> f32 { return max_button_width; }
    };
}
