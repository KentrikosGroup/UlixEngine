#pragma once

#include "callback.hpp"
#include "cursor.hpp"
#include "file.hpp"
#include "font.hpp"
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
            using bitmap = ulx::file;
            using vector = ulx::pack<ulx::file, ulx::size>;
            using text = ulx::pack<ulx::u8char, ulx::font>;
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
        ulx::color background_color = ulx::color(255, 255, 255, 255);
        ulx::color border_color = ulx::color(255, 255, 255, 255);
        ulx::f32 corner_radius = 16.0f;
        ulx::f32 border_width = 3.0f;
        ulx::point p = ulx::point(0, 0);
        ulx::size siz = ulx::size(200, 200);
        ulx::vec<ulx::element> objects;
        ulx::u8 alignment = ulx::align::center;
        ulx::f32 paddin = 5;
        ulx::box bx = ulx::box::nonebox;
        texture::variant texturev;
        ulx::strvw id_;
        event::released released_e;
        event::pressed pressed_e;
        event::hovered hovered_e;
        ulx::cursor curs = ulx::cursor::resetcur;

        public: inline constexpr element() = default;
        public: inline constexpr element(ulx::strvw id): id_(id) {}

        public: inline constexpr auto id(ulx::strvw id) -> element& {
            this->id_ = id;
            return *this;
        }

        public: inline constexpr auto style(const ulx::style& style) -> element& {
            background_color = style.background;
            border_color = style.border;
            corner_radius = style.corner;
            border_width = style.borderw;
            return *this;
        }

        public: inline constexpr auto back(const ulx::color& color) -> element& {
            background_color = color;
            return *this;
        }

        public: inline constexpr auto border(const ulx::color& color) -> element& {
            border_color = color;
            return *this;
        }

        public: inline constexpr auto corner(ulx::f32 radius) -> element& {
            corner_radius = radius;
            return *this;
        }

        public: inline constexpr auto border(ulx::f32 width) -> element& {
            border_width = width;
            return *this;
        }

        public: inline constexpr auto size(const ulx::size& siz) -> element& {
            this->siz = siz;
            return *this;
        }

        public: inline constexpr auto pos(const ulx::point& p) -> element& {
            this->p = ulx::point(p.x(), p.y());
            return *this;
        }

        public: inline constexpr auto child(const element& obj) -> element& {
            objects.push_back(obj);
            return *this;
        }

        public: inline constexpr auto texture(const file& file) -> element& {
            log::expect(file.exists(), "element.hpp: namespace ulx: class element: texture(const ulx::file&): file.exists() == false");
            texturev.emplace<texture::bitmap>(file);
            return *this;
        }

        public: inline constexpr auto texture(const file& svg, const ulx::size& size) -> element& {
            log::expect(svg.exists(), "element.hpp: namespace ulx: class element: texture(const ulx::file&, const ulx::size&): svg.exists() == false");
            texturev.emplace<texture::vector>(svg, size);
            return *this;
        }

        public: inline constexpr auto texture(const ulx::u8char& chr, const ulx::font& font = ulx::font()) -> element& {
            texturev.emplace<texture::text>(chr, font);
            return *this;
        }

        public: inline constexpr auto align(ulx::u8 alignment) -> element& {
            this->alignment = alignment;
            return *this;
        }

        public: inline constexpr auto pad(ulx::f32 padding) -> element& {
            paddin = padding;
            return *this;
        }

        public: inline constexpr auto box(ulx::box bx) -> element& {
            this->bx = bx;
            return *this;
        }

        template<typename _released>
            requires ulx::expect<_released, event::released>
        inline constexpr auto released(_released&& event) -> element& {
            released_e = std::forward<_released>(event);
            return *this;
        }

        template<typename _pressed>
            requires ulx::expect<_pressed, event::pressed>
        inline constexpr auto pressed(_pressed&& event) -> element& {
            pressed_e = std::forward<_pressed>(event);
            return *this;
        }

        template<typename _hovered>
            requires ulx::expect<_hovered, event::hovered>
        inline constexpr auto hovered(_hovered&& event) -> element& {
            hovered_e = std::forward<_hovered>(event);
            return *this;
        }

        inline constexpr auto layout(const layoutbox& layout) -> element& {
            bx = layout.box;
            paddin = layout.padding;
            alignment = layout.align;
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
            curs = cursor;
            return *this;
        }

        inline constexpr auto geom(const geom& geom) -> element& {
            p = ulx::point(geom.pos.x(), geom.pos.y() + 0.5);
            siz = geom.size;
            return *this;
        }

    public:
        inline constexpr auto get_background_color() const -> const color& { return background_color; }
        inline constexpr auto get_border_color() const -> const color& { return border_color; }
        inline constexpr auto get_corner_radius() const -> f32 { return corner_radius; }
        inline constexpr auto get_border_width() const -> f32 { return border_width; }
        inline constexpr auto get_size() const -> const ulx::size& { return siz; }
        inline constexpr auto get_pos() const -> const point& { return p; }
        inline constexpr auto get_objects() const -> const vec<ulx::element>& { return objects; }
        inline constexpr auto get_texture() const -> const texture::variant& { return texturev; }
        inline constexpr auto get_alignment() const -> u8 { return alignment; }
        inline constexpr auto get_padding() const -> f32 { return paddin; }
        inline constexpr auto get_layout() const -> ulx::box { return bx; }
        inline constexpr auto get_id() const -> const strvw& { return id_; }
        inline constexpr auto get_released_event() const -> const event::released& { return released_e; }
        inline constexpr auto get_pressed_event() const -> const event::pressed& { return pressed_e; }
        inline constexpr auto get_hovered_event() const -> const event::hovered& { return hovered_e; }
        inline constexpr auto get_cursor() const -> ulx::cursor { return curs; }

    public:
        inline static constexpr auto box() -> element {
            return element().size(ulx::size(0, 0));
        }

        template<typename _font = font>
            requires ulx::expect<_font, font>
        inline static constexpr auto text(const str& txt, _font&& font = ulx::font()) -> element {
            auto metrics = __uii::fgg::line_metrics(font);
            f32 height = static_cast<f32>(ulx::wfn::logical(metrics.template get<2>(), window_dpi));
            element obj = box().size({0, height})
                .align(ulx::alignleft | ulx::aligntop)
                .box(ulx::horbox).pad(0);

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

                obj.child(element(glyph_str.cstr())
                    .texture(glyph_str, font)
                    .size(ulx::size::autos())
                    .border(0).corner(0).back(ulx::color::empty())
                );
            }

            return obj;
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
