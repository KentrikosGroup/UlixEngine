#pragma once
#include "requires.hpp"
#include "size.hpp"
#include "point.hpp"
#include "stablestr.hpp"
#include "winattr.hpp"
#include "types.hpp"
#include <utility>

namespace ulx {
    class wininfo {
        u8 attrs = winattr::normalattr;
        stablestr<> title;
        size inital_size = {800, 600};
        point initial_pos = point(0, 0);
        size min_size = {800, 600};
        size max_size = {1200, 900};
        f32 titlebarh = 30;
        f32 closebtnw = 45;
        f32 minbtnw = 45;
        f32 maxbtnw = 45;

        public: inline constexpr wininfo() = default;

        public: template<typename t_title>
            requires ulx::expect<t_title, stablestr<>>
        inline constexpr wininfo(t_title&& title): title(std::forward<t_title>(title)) {}

        inline constexpr auto size(const ulx::size& rec) -> wininfo& { inital_size = rec; return *this; }
        inline constexpr auto pos(const point& rec) -> wininfo& { initial_pos = rec; return *this; }
        inline constexpr auto attr(u8 attr) -> wininfo& { attrs |= attr; return *this; }
        inline constexpr auto unattr(u8 attr) -> wininfo& { attrs &= ~attr; return *this; }
        inline constexpr auto min(const ulx::size& rec) -> wininfo& { min_size = rec; return *this; }
        inline constexpr auto max(const ulx::size& rec) -> wininfo& { max_size = rec; return *this; }
        inline constexpr auto vtitlebar(f32 height) -> wininfo& { titlebarh = height; return *this; }
        inline constexpr auto vclose(f32 width) -> wininfo& { closebtnw = width; return *this; }
        inline constexpr auto vmin(f32 width) -> wininfo& { minbtnw = width; return *this; }
        inline constexpr auto vmax(f32 width) -> wininfo& { maxbtnw = width; return *this; }

    public:
        inline constexpr auto get_window_attrs() const -> u8 { return attrs; }
        inline constexpr auto get_window_title() const -> const stablestr<>& { return title; }
        inline constexpr auto get_initial_window_size() const -> const ulx::size& { return inital_size; }
        inline constexpr auto get_initial_window_pos() const -> const point& { return initial_pos; }
        inline constexpr auto get_min_window_size() const -> const ulx::size& { return min_size; }
        inline constexpr auto get_max_window_size() const -> const ulx::size& { return max_size; }
        inline constexpr auto get_titlebar_height() const -> f32 { return titlebarh; }
        inline constexpr auto get_close_button_width() const -> f32 { return closebtnw; }
        inline constexpr auto get_min_button_width() const -> f32 { return minbtnw; }
        inline constexpr auto get_max_button_width() const -> f32 { return maxbtnw; }
    };
}
