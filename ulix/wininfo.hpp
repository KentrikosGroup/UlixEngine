#pragma once
#include <ulix/requires.hpp>
#include <ulix/size.hpp>
#include <ulix/point.hpp>
#include <ulix/stablestr.hpp>
#include <ulix/winattr.hpp>
#include <ulix/types.hpp>

namespace ulx {
    class wininfo {
        stablestr<> m_title;
        u8          m_attrs = winattr::normalattr;
        size        m_initalsize = {800, 600};
        point       m_initialpos = point(0, 0);
        size        m_minsize = {800, 600},
                    m_maxsize = {1200, 900};
        f32         m_titlebar_h = 30,
                    m_closebtn_w = 45,
                    m_minbtn_w = 45,
                    m_maxbtn_w = 45;

        public: inline constexpr wininfo() = default;
        public: template<typename t_title>
            requires ulx::expect<t_title, stablestr<>>
        inline constexpr wininfo(t_title&& title): m_title(std::forward<t_title>(title)) {}

        public: inline constexpr auto size(const ulx::size& rec) -> wininfo& { m_initalsize = rec; return *this; }
        public: inline constexpr auto pos(const point& rec) -> wininfo& { m_initialpos = rec; return *this; }
        public: inline constexpr auto attr(u8 attr) -> wininfo& { m_attrs |= attr; return *this; }
        public: inline constexpr auto unattr(u8 attr) -> wininfo& { m_attrs &= ~attr; return *this; }
        public: inline constexpr auto min(const ulx::size& rec) -> wininfo& { m_minsize = rec; return *this; }
        public: inline constexpr auto max(const ulx::size& rec) -> wininfo& { m_maxsize = rec; return *this; }
        public: inline constexpr auto vtitlebar(f32 height) -> wininfo& { m_titlebar_h = height; return *this; }
        public: inline constexpr auto vclose(f32 width) -> wininfo& { m_closebtn_w = width; return *this; }
        public: inline constexpr auto vmin(f32 width) -> wininfo& { m_minbtn_w = width; return *this; }
        public: inline constexpr auto vmax(f32 width) -> wininfo& { m_maxbtn_w = width; return *this; }

        public: inline constexpr auto get_window_attrs() const -> u8 { return m_attrs; }
        public: inline constexpr auto get_window_title() const -> const stablestr<>& { return m_title; }
        public: inline constexpr auto get_initial_window_size() const -> const ulx::size& { return m_initalsize; }
        public: inline constexpr auto get_initial_window_pos() const -> const point& { return m_initialpos; }
        public: inline constexpr auto get_min_window_size() const -> const ulx::size& { return m_minsize; }
        public: inline constexpr auto get_max_window_size() const -> const ulx::size& { return m_maxsize; }
        public: inline constexpr auto get_titlebar_height() const -> f32 { return m_titlebar_h; }
        public: inline constexpr auto get_close_button_width() const -> f32 { return m_closebtn_w; }
        public: inline constexpr auto get_min_button_width() const -> f32 { return m_minbtn_w; }
        public: inline constexpr auto get_max_button_width() const -> f32 { return m_maxbtn_w; }
    };
}
