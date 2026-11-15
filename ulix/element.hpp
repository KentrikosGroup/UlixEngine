#pragma once

#include "callback.hpp"
#include "cursor.hpp"
#include "font.hpp"
#include "glm/ext/vector_float4.hpp"
#include "log.hpp"
#include "box.hpp"
#include "point.hpp"
#include "requires.hpp"
#include "size.hpp"
#include "color.hpp"
#include "align.hpp"
#include "pack.hpp"
#include "types.hpp"
#include "ulix/path.hpp"
#include <utility>
#include <variant>

#define ulx_lambda_when (ulx::element& slf)
#define ulx_lambda_else (ulx::element& slf)

namespace ulx {
    class context;
    class element {
        public: struct texture {
            using none = std::monostate;
            using bitmap = path;
            using vector = pack<path, size>;
            using text = pack<u8char, font>;
            using variant = std::variant<none, bitmap, vector, text>;
        };

        public: struct event {
            using callback = ulx::callback<void(element&)>;
            using mouse = ulx::callback<void(context&, element&)>;
            using released = mouse;
            using pressed = mouse;
            using hovered = mouse;
        };

        private:
        strvw        m_id;
        vec<element> m_elements;
        vec<element> m_shadows;

        point m_pos = {0, 0};
        size  m_size = {0, 0};
        u8    m_align = aligndef;
        f32   m_padding = 0;
        box   m_box = nonbox;

        color m_back_color = color::empty();
        color m_border_color = color::empty();
        f32   m_corner_radius = 0;
        f32   m_border_width = 0;

        event::released m_released;
        event::pressed  m_pressed;
        event::hovered  m_hovered;

        texture::variant m_texture;
        cursor    m_curs      = arrowcur;
        glm::vec4 m_data      = {1.0f, 0, 1, 0};
        bool      m_is_shadow = false;

        public: inline constexpr element() = default;
        public: inline constexpr element(strvw id): m_id(id) {}

        public: inline constexpr auto id(strvw id) -> element& {
            this->m_id = id;
            return *this;
        }

        public: inline constexpr auto back(const color& color) -> element& {
            m_back_color = color;
            return *this;
        }

        public: inline constexpr auto border(const color& color) -> element& {
            m_data = color.to_vec4();
            return *this;
        }

        public: inline constexpr auto corner(f32 radius) -> element& {
            m_corner_radius = radius;
            return *this;
        }

        public: inline constexpr auto border(f32 width) -> element& {
            m_border_width = width;
            return *this;
        }

        public: inline constexpr auto size(const size& siz) -> element& {
            this->m_size = siz;
            return *this;
        }

        public: inline constexpr auto pos(const point& p) -> element& {
            this->m_pos = point(p.x(), p.y());
            return *this;
        }

        public: inline constexpr auto child(const element& obj) -> element& {
            m_elements.push_back(obj);
            return *this;
        }

        public: template<typename _path>
            requires expect<_path, path>
        inline constexpr auto texture(_path&& bitmap, f32 sigma = 1.0f, u32 radius = 0, u32 taps = 1) -> element& {
            log::expect(bitmap.exists(), "element.hpp: namespace ulx: class element: texture(_path&&): bitmap.exists() == false");
            m_texture.emplace<texture::bitmap>(std::forward<_path>(bitmap));
            m_data = {sigma, radius, taps, 0};

            return *this;
        }

        public: template<typename _path, typename _size>
            requires expect<_path, path> && expect<_size, ulx::size>
        inline constexpr auto texture(_path&& svg, _size&& size) -> element& {
            log::expect(svg.exists(), "element.hpp: namespace ulx: class element: texture(_path&&, _size&&): svg.exists() == false");
            m_texture.emplace<texture::vector>(std::forward<_path>(svg), std::forward<_size>(size));
            return *this;
        }

        public: template<typename _path>
            requires expect<_path, path>
        inline constexpr auto icon(strvw id, _path&& svg, const ulx::size& s) -> element& {
            return child(element()
                    .back(color::empty())
                    .size(s)
                    .pos(point::center_to(s, m_size))
                    .texture(std::forward<_path>(svg), s));
        }

        public: template<typename _font>
            requires expect<_font, font>
        inline constexpr auto texture(u8char chr, _font&& font = {}) -> element& {
            m_texture.emplace<texture::text>(chr, std::forward<_font>(font));
            return *this;
        }

        public: inline constexpr auto align(u8 alignment) -> element& {
            this->m_align = alignment;
            return *this;
        }

        public: inline constexpr auto pad(f32 padding) -> element& {
            m_padding = padding;
            return *this;
        }

        public: inline constexpr auto box(box bx) -> element& {
            this->m_box = bx;
            return *this;
        }

        public: template<typename _released>
            requires expect<_released, event::released>
        inline constexpr auto released(_released&& event) -> element& {
            m_released = std::forward<_released>(event);
            return *this;
        }

        public: template<typename _pressed>
            requires expect<_pressed, event::pressed>
        inline constexpr auto pressed(_pressed&& event) -> element& {
            m_pressed = std::forward<_pressed>(event);
            return *this;
        }

        public: template<typename _hovered>
            requires expect<_hovered, event::hovered>
        inline constexpr auto hovered(_hovered&& event) -> element& {
            m_hovered = std::forward<_hovered>(event);
            return *this;
        }

        public: inline constexpr auto call(const event::callback& fn) -> element& {
            fn(*this);
            return *this;
        }

        public: inline constexpr auto when(bool cond, const event::callback& fn) -> element& {
            if (cond)
                fn(*this);
            return *this;
        }

        public: inline constexpr auto when(bool cond, const event::callback& fn, const event::callback& else_fn) -> element& {
            if (cond) fn(*this);
            else else_fn(*this);
            return *this;
        }

        public: inline constexpr auto cur(cursor cursor) -> element& {
            m_curs = cursor;
            return *this;
        }

        public: inline constexpr auto shadow(const color& color, f32 radius, f32 width, const point& offset = {0, 0}, f32 exp = 2.0f, bool inset = false) -> element& {
            auto c = ulx::element("shadow")
                .pos({-width + offset.x(), -width + offset.y()})
                .corner(radius)
                .border(width)
                .back(color);
            c.m_is_shadow = true;
            c.m_data = {f32(inset), exp, 0, 0};
            m_shadows.push_back(c);
            
            return *this;
        }

        public: template<typename t_vec4>
            requires expect<t_vec4, glm::vec4>
        inline constexpr auto data(t_vec4&& data) -> element& {
            m_data = std::forward<t_vec4>(data);
            return *this;
        }

        public: inline constexpr auto get_background_color() const -> const color& { return m_back_color; }
        public: inline constexpr auto get_border_color() const -> const color& { return m_border_color; }
        public: inline constexpr auto get_corner_radius() const -> f32 { return m_corner_radius; }
        public: inline constexpr auto get_border_width() const -> f32 { return m_border_width; }
        public: inline constexpr auto get_size() const -> const ulx::size& { return m_size; }
        public: inline constexpr auto get_pos() const -> const point& { return m_pos; }
        public: inline constexpr auto get_objects() const -> const vec<ulx::element>& { return m_elements; }
        public: inline constexpr auto get_texture() const -> const texture::variant& { return m_texture; }
        public: inline constexpr auto get_alignment() const -> u8 { return m_align; }
        public: inline constexpr auto get_padding() const -> f32 { return m_padding; }
        public: inline constexpr auto get_layout() const -> ulx::box { return m_box; }
        public: inline constexpr auto get_id() const -> const strvw& { return m_id; }
        public: inline constexpr auto get_released_event() const -> const event::released& { return m_released; }
        public: inline constexpr auto get_pressed_event() const -> const event::pressed& { return m_pressed; }
        public: inline constexpr auto get_hovered_event() const -> const event::hovered& { return m_hovered; }
        public: inline constexpr auto get_cursor() const -> ulx::cursor { return m_curs; }
        public: inline constexpr auto get_data() const -> const glm::vec4& { return m_data; }
        public: inline constexpr auto is_shadow() const -> bool { return m_is_shadow; }
        public: inline constexpr auto get_shadows() -> vec<element>& { return m_shadows; }
    };
}

namespace std {
    template<> struct hash<ulx::element::texture::vector> {
        size_t operator()(const ulx::element::texture::vector& v) const noexcept {
            return make_hash(v.get<0>(), v.get<1>());
        }
    };

    template<> struct hash<ulx::element::texture::text> {
        size_t operator()(const ulx::element::texture::text& t) const noexcept {
            return make_hash(t.get<0>(), t.get<1>());
        }
    };
}
