#pragma once

#include "rect.hpp"
#include "winattr.hpp"
#include "types.hpp"

namespace ulx {
    class wininfo {
        private:
            ulx::u8 window_attrs = ulx::winattr::Normal;
            ulx::str window_class_name;
            ulx::str window_title;
            ulx::rect initial_window_rect = ulx::rect(0, 0, 800, 600);
            ulx::rect min_window_rect = ulx::rect(0, 0, 800, 600);
            ulx::rect max_window_rect = ulx::rect(0, 0, 1200, 900);
        
        public:
            wininfo() = default;
            inline constexpr wininfo(const ulx::str& window_title, const ulx::str& window_class_name = "UlixWindowClass"):
                window_class_name(window_class_name),
                window_title(window_title)
            {}
        
            inline auto rect(const ulx::rect& rec) -> wininfo& { initial_window_rect = rec; return *this; }
            inline auto attrs(ulx::u8 attrs) -> wininfo& { window_attrs |= attrs; return *this; }
            inline auto unattrs(ulx::u8 attrs) -> wininfo& { window_attrs &= ~attrs; return *this; }
            inline auto min_rect(const ulx::rect& rec) -> wininfo& { min_window_rect = rec; return *this; }
            inline auto max_rect(const ulx::rect& rec) -> wininfo& { max_window_rect = rec; return *this; }
        
        public:
            inline auto get_window_attrs() const -> ulx::u8 { return window_attrs; }
            inline auto get_window_class_name() const -> const ulx::str& { return window_class_name; }
            inline auto get_window_title() const -> const ulx::str& { return window_title; }
            inline auto get_initial_window_rect() const -> ulx::rect { return initial_window_rect; }
            inline auto get_min_window_rect() const -> ulx::rect { return min_window_rect; }
            inline auto get_max_window_rect() const -> ulx::rect { return max_window_rect; }
    };
}
