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
#include "__inside_impl/font_glyph_getter.hpp"
#include "types.hpp"
#include "ulix/__inside_impl/vulkan_algorithm.hpp"
#include "ulix/path.hpp"
#include "ulix/winfunc.hpp"
#include <utility>
#include <variant>

#define event_when
#define event_else

namespace ulx {
    inline static UINT window_dpi = 0;

    struct style {
        ulx::color background = ulx::color(255, 255, 255, 255);
        ulx::color border = ulx::color(255, 255, 255, 255);
        ulx::f32 corner = 16.0f;
        ulx::f32 borderw = 16.0f;
    };

    struct layoutbox {
        ulx::box box = ulx::box::nonebox;
        ulx::u8 align = ulx::align::alignleft | ulx::align::aligntop;
        ulx::f32 padding = 0.0f;
    };

    struct geom {
        ulx::size size = ulx::size(0, 0);
        ulx::point pos = ulx::point(0, 0);
    };

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
        ulx::color m_back_color = ulx::color::empty();
        ulx::color m_border_color = ulx::color::empty();
        ulx::f32 m_corner_radius = 0;
        ulx::f32 m_border_width = 0;
        ulx::point m_pos = ulx::point(0, 0);
        ulx::size m_size = ulx::size(0, 0);
        ulx::vec<ulx::element> m_elements;
        ulx::u8 m_align = ulx::align::center;
        ulx::f32 m_padding = 5;
        ulx::box m_box = ulx::box::nonebox;
        texture::variant m_texture;
        ulx::strvw m_id;
        event::released m_released;
        event::pressed m_pressed;
        event::hovered m_hovered;
        ulx::cursor m_curs = ulx::cursor::resetcur;
        glm::vec4 m_data = {0, 0, 0, 0};

        public: inline constexpr element() = default;
        public: inline constexpr element(strvw id): m_id(id) {}
        public: inline constexpr element(strvw id, const str& txt, const font& font = ulx::font(), const color& color = ulx::color(255, 255, 255, 255)): m_id(id) {
            auto metrics = __uii::fgg::line_metrics(font);
            f32 height = static_cast<f32>(ulx::wfn::logical(metrics.template get<2>(), window_dpi));

            size({0, height});
            align(ulx::alignleft | ulx::aligntop);
            box(ulx::horbox).pad(0);

            for (ulx::u64 i = 0; i < txt.size(); ) {
                ulx::u32 cp = __uii::fgg::decode_utf8(txt, i);
                ulx::u8char glyph_str;

                if (cp < 0x80) {
                    glyph_str += static_cast<char>(cp);
                } else if (cp < 0x800) {
                    glyph_str += static_cast<char>(0xC0 | (cp >> 6));
                    glyph_str += static_cast<char>(0x80 | (cp & 0x3F));
                } else {
                    glyph_str += static_cast<char>(0xE0 | (cp >> 12));
                    glyph_str += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    glyph_str += static_cast<char>(0x80 | (cp & 0x3F));
                }

                auto c = element(glyph_str.cstr())
                    .texture(glyph_str, font)
                    .size(ulx::size::autos())
                    .border(0).corner(0).back(ulx::color::empty());
                c.m_data = {color.red01(), color.green01(), color.blue01(), color.alpha01()};

                child(c);
            }
        }

        public: inline constexpr auto id(strvw id) -> element& {
            this->m_id = id;
            return *this;
        }

        public: inline constexpr auto style(const ulx::style& style) -> element& {
            m_back_color = style.background;
            m_data = __uii::vkalg::color_to_vec4(style.border);
            m_corner_radius = style.corner;
            m_border_width = style.borderw;
            return *this;
        }

        public: inline constexpr auto back(const ulx::color& color) -> element& {
            m_back_color = color;
            return *this;
        }

        public: inline constexpr auto border(const ulx::color& color) -> element& {
            m_data = __uii::vkalg::color_to_vec4(color);
            return *this;
        }

        public: inline constexpr auto corner(ulx::f32 radius) -> element& {
            m_corner_radius = radius;
            return *this;
        }

        public: inline constexpr auto border(ulx::f32 width) -> element& {
            m_border_width = width;
            return *this;
        }

        public: inline constexpr auto size(const ulx::size& siz) -> element& {
            this->m_size = siz;
            return *this;
        }

        public: inline constexpr auto pos(const ulx::point& p) -> element& {
            this->m_pos = ulx::point(p.x(), p.y());
            return *this;
        }

        public: inline constexpr auto child(const element& obj) -> element& {
            m_elements.push_back(obj);
            return *this;
        }

        public: template<typename _path>
            requires ulx::expect<_path, path>
        inline constexpr auto texture(_path&& bitmap, f32 sigma = 1.0f, u32 radius = 0, u32 taps = 1) -> element& {
            log::expect(bitmap.exists(), "element.hpp: namespace ulx: class element: texture(_path&&): bitmap.exists() == false");
            m_texture.emplace<texture::bitmap>(std::forward<_path>(bitmap));
            m_data = {sigma, radius, taps, 0};

            return *this;
        }

        public: template<typename _path, typename _size>
            requires ulx::expect<_path, path> && ulx::expect<_size, ulx::size>
        inline constexpr auto texture(_path&& svg, _size&& size) -> element& {
            log::expect(svg.exists(), "element.hpp: namespace ulx: class element: texture(_path&&, _size&&): svg.exists() == false");
            m_texture.emplace<texture::vector>(std::forward<_path>(svg), std::forward<_size>(size));
            return *this;
        }

        public: template<typename _path>
            requires ulx::expect<_path, path>
        inline constexpr auto icon(strvw id, _path&& svg, const ulx::size& s) -> element& {
            child(element().style({
                .background = ulx::color::empty(),
                .corner = 0, .borderw = 0
            }).size(s).pos(ulx::point::center_to(s, m_size)).texture(std::forward<_path>(svg), s));
            return *this;
        }

        public: template<typename _font>
            requires ulx::expect<_font, font>
        inline constexpr auto texture(u8char chr, _font&& font = ulx::font()) -> element& {
            m_texture.emplace<texture::text>(chr, std::forward<_font>(font));
            return *this;
        }

        public: inline constexpr auto align(ulx::u8 alignment) -> element& {
            this->m_align = alignment;
            return *this;
        }

        public: inline constexpr auto pad(ulx::f32 padding) -> element& {
            m_padding = padding;
            return *this;
        }

        public: inline constexpr auto box(ulx::box bx) -> element& {
            this->m_box = bx;
            return *this;
        }

        template<typename _released>
            requires ulx::expect<_released, event::released>
        inline constexpr auto released(_released&& event) -> element& {
            m_released = std::forward<_released>(event);
            return *this;
        }

        template<typename _pressed>
            requires ulx::expect<_pressed, event::pressed>
        inline constexpr auto pressed(_pressed&& event) -> element& {
            m_pressed = std::forward<_pressed>(event);
            return *this;
        }

        template<typename _hovered>
            requires ulx::expect<_hovered, event::hovered>
        inline constexpr auto hovered(_hovered&& event) -> element& {
            m_hovered = std::forward<_hovered>(event);
            return *this;
        }

        inline constexpr auto layout(const layoutbox& layout) -> element& {
            m_box = layout.box;
            m_padding = layout.padding;
            m_align = layout.align;
            return *this;
        }

        inline constexpr auto call(const event::callback& fn) -> element& {
            fn(*this);
            return *this;
        }

        inline constexpr auto when(bool cond, const event::callback& fn) -> element& {
            if (cond)
                fn(*this);
            return *this;
        }

        inline constexpr auto when(bool cond, const event::callback& fn, const event::callback& else_fn) -> element& {
            if (cond) fn(*this);
            else else_fn(*this);
            return *this;
        }

        inline constexpr auto cur(ulx::cursor cursor) -> element& {
            m_curs = cursor;
            return *this;
        }

        inline constexpr auto geom(const geom& geom) -> element& {
            m_pos = ulx::point(geom.pos.x(), geom.pos.y() + 0.5);
            m_size = geom.size;
            return *this;
        }

    public:
        inline constexpr auto get_background_color() const -> const color& { return m_back_color; }
        inline constexpr auto get_border_color() const -> const color& { return m_border_color; }
        inline constexpr auto get_corner_radius() const -> f32 { return m_corner_radius; }
        inline constexpr auto get_border_width() const -> f32 { return m_border_width; }
        inline constexpr auto get_size() const -> const ulx::size& { return m_size; }
        inline constexpr auto get_pos() const -> const point& { return m_pos; }
        inline constexpr auto get_objects() const -> const vec<ulx::element>& { return m_elements; }
        inline constexpr auto get_texture() const -> const texture::variant& { return m_texture; }
        inline constexpr auto get_alignment() const -> u8 { return m_align; }
        inline constexpr auto get_padding() const -> f32 { return m_padding; }
        inline constexpr auto get_layout() const -> ulx::box { return m_box; }
        inline constexpr auto get_id() const -> const strvw& { return m_id; }
        inline constexpr auto get_released_event() const -> const event::released& { return m_released; }
        inline constexpr auto get_pressed_event() const -> const event::pressed& { return m_pressed; }
        inline constexpr auto get_hovered_event() const -> const event::hovered& { return m_hovered; }
        inline constexpr auto get_cursor() const -> ulx::cursor { return m_curs; }
        inline constexpr auto get_data() const -> const glm::vec4& { return m_data; }

    public:
        inline static constexpr auto box() -> element {
            return element().size(ulx::size(0, 0));
        }

        template<typename _geom>
            requires ulx::expect<_geom, ulx::geom>
        inline static constexpr auto unvisible(ulx::strvw id, _geom&& g) -> element {
            return element(id)
                .style({
                    .background = ulx::color::empty(),
                    .corner = 0, .borderw = 0
                }).geom(std::forward<_geom>(g));
        }

        inline static constexpr auto unbordered(ulx::strvw id) -> element {
            return element(id).corner(0).border(0);
        }
    };
}

namespace std {
    template<>
    struct hash<ulx::element::texture::vector> {
        size_t operator()(const ulx::element::texture::vector& v) const noexcept {
            return make_hash(v.get<0>(), v.get<1>());
        }
    };

    template<>
    struct hash<ulx::element::texture::text> {
        size_t operator()(const ulx::element::texture::text& t) const noexcept {
            return make_hash(t.get<0>(), t.get<1>());
        }
    };
}
