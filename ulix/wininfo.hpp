#pragma once

#include "rect.hpp"
#include "recsize.hpp"
#include "recpos.hpp"
#include "winattr.hpp"
#include "types.hpp"
#include "requires.hpp"
#include <utility>

namespace ulx {
    class wininfo {
        private:
            ulx::u8 window_attrs = ulx::winattr::normal;
            ulx::str window_class_name;
            ulx::str window_title;
            ulx::recsize initial_window_size = ulx::recsize(800, 600);
            ulx::recpos initial_window_pos = ulx::recpos(0, 0);
            ulx::recsize min_window_size = ulx::recsize(800, 600);
            ulx::recsize max_window_size = ulx::recsize(1200, 900);
        
        public:
            inline constexpr wininfo() = default;

            template<typename T, typename CN> 
                requires ulx::expect<T, ulx::str> && ulx::expect<CN, ulx::str>
            inline constexpr wininfo(
                T&& window_title, 
                CN&& window_class_name = "UlixWindowClass"
            ):
                window_class_name(std::forward<CN>(window_class_name)),
                window_title(std::forward<T>(window_title))
            {}
        
            inline auto size(const ulx::recsize& rec) -> wininfo& { initial_window_size = rec; return *this; }
            inline auto pos(const ulx::recpos& rec) -> wininfo& { initial_window_pos = rec; return *this; }
            inline auto attr(ulx::u8 attrs) -> wininfo& { window_attrs |= attrs; return *this; }
            inline auto unattr(ulx::u8 attrs) -> wininfo& { window_attrs &= ~attrs; return *this; }
            inline auto min(const ulx::recsize& rec) -> wininfo& { min_window_size = rec; return *this; }
            inline auto max(const ulx::recsize& rec) -> wininfo& { max_window_size = rec; return *this; }
        
        public:
            inline auto get_window_attrs() const -> ulx::u8 { return window_attrs; }
            inline auto get_window_class_name() const -> const ulx::str& { return window_class_name; }
            inline auto get_window_title() const -> const ulx::str& { return window_title; }
            inline auto get_initial_window_size() const -> const ulx::recsize& { return initial_window_size; }
            inline auto get_initial_window_pos() const -> const ulx::recpos& { return initial_window_pos; }
            inline auto get_min_window_size() const -> const ulx::recsize& { return min_window_size; }
            inline auto get_max_window_size() const -> const ulx::recsize& { return max_window_size; }
    };
}
