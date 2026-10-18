#pragma once

#include "callback.hpp"
#include "cursor.hpp"
#include "file.hpp"
#include "font.hpp"
#include "log.hpp"
#include "box.hpp"
#include "point.hpp"
#include "size.hpp"
#include "color.hpp"
#include "align.hpp"
#include "requires.hpp"
#include "__inside_impl/fontloader.hpp"
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
        public:
            struct texture {
                using none = std::monostate;
                using bitmap = ulx::file;
                using vector = ulx::pack<ulx::file, ulx::size>;
                using text = ulx::pack<ulx::str, ulx::font>;
                using variant = std::variant<none, bitmap, vector, text>;
            };
            
            struct event {
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

        public:
            inline constexpr element() = default;

            template<typename S>
                requires ulx::expect<S, ulx::strvw>
            inline constexpr element(S&& id): id_(std::forward<S>(id)) {}

        public:
            template<typename S>
                requires ulx::expect<S, ulx::strvw>
            inline constexpr auto id(S&& id) -> element& {
                this->id_ = std::forward<S>(id);
                return *this;
            }

            inline constexpr auto style(const ulx::style& style) -> element& {
                background_color = style.background;
                border_color = style.border;
                corner_radius = style.corner;
                border_width = style.borderw;
                return *this;
            }

            inline constexpr auto back(const ulx::color& color) -> element& {
                background_color = color;
                return *this;
            }

            inline constexpr auto border(const ulx::color& color) -> element& {
                border_color = color;
                return *this;
            }

            inline constexpr auto corner(ulx::f32 radius) -> element& {
                corner_radius = radius;
                return *this;
            }

            inline constexpr auto border(ulx::f32 width) -> element& {
                border_width = width;
                return *this;
            }

            inline constexpr auto size(const ulx::size& siz) -> element& {
                this->siz = siz;
                return *this;
            }

            inline constexpr auto pos(const ulx::point& p) -> element& {
                this->p = ulx::point(p.x(), p.y());
                return *this;
            }

            inline constexpr auto child(const element& obj) -> element& {
                objects.push_back(obj);
                return *this;
            }

            inline constexpr auto texture(const ulx::file& bitmap_or_svg) -> element& {
                if (!bitmap_or_svg.exists()) ulx::log::err("{} file {} doesn't exists", bitmap_or_svg.path());
                texturev = bitmap_or_svg;
                return *this;
            }

            inline constexpr auto texture(const ulx::file& svg, const ulx::size& size) -> element& {
                if (!svg.exists()) ulx::log::err("SVG file {} doesn't exists", svg.path());
                texturev = ulx::pack<file, ulx::size>(svg, size);
                return *this;
            }

            inline constexpr auto texture(const ulx::str& text, const ulx::font& font = ulx::font()) -> element& {
                texturev = ulx::pack<str, ulx::font>(text, font);
                return *this;
            }

            inline constexpr auto align(ulx::u8 alignment) -> element& {
                this->alignment = alignment;
                return *this;
            }

            inline constexpr auto pad(ulx::f32 padding) -> element& {
                paddin = padding;
                return *this;
            }

            inline constexpr auto box(ulx::box bx) -> element& {
                this->bx = bx;
                return *this;
            }

            template<typename R>
                requires ulx::expect<R, event::released>
            inline constexpr auto released(R&& event) -> element& {
                released_e = std::forward<R>(event);
                return *this;
            }

            template<typename P>
                requires ulx::expect<P, event::pressed>
            inline constexpr auto pressed(P&& event) -> element& {
                pressed_e = std::forward<P>(event);
                return *this;
            }

            template<typename H>
                requires ulx::expect<H, event::hovered>
            inline constexpr auto hovered(H&& event) -> element& {
                hovered_e = std::forward<H>(event);
                return *this;
            }

            inline constexpr auto layout(const layoutbox& layout) -> element& {
                bx = layout.box;
                paddin = layout.padding;
                alignment = layout.align;
                return *this;
            }

            template<typename C>
                requires ulx::expect<C, event::callback>
            inline constexpr auto call(C&& fn) -> element& {
                std::forward<C>(fn)(*this);
                return *this;
            }

            template<typename C>
                requires ulx::expect<C, event::callback>
            inline constexpr auto when(bool cond, C&& fn) -> element& {
                if (cond)
                    std::forward<C>(fn)(*this);
                return *this;
            }

            template<typename C, typename E>
                requires ulx::expect<C, event::callback> &&
                         ulx::expect<E, event::callback>
            inline constexpr auto when(bool cond, C&& fn, E&& else_fn) -> element& {
                if (cond)
                    std::forward<C>(fn)(*this);
                else
                    std::forward<E>(else_fn)(*this);
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

            template<typename F>
                requires ulx::expect<F, ulx::font>
            inline static constexpr auto text(const ulx::str& txt, ulx::align align = ulx::align::alignbottom, ulx::identity<F>&& font = ulx::font()) -> element {
                ulx::i32 max_h = __uii::font::max_height_of(font);
                element obj = box().pad(0)
                    .align(ulx::alignleft | align)
                    .box(ulx::horbox).size(ulx::size(0, max_h));

                ulx::str glyph_str;
                for (ulx::u64 i = 0; i < txt.size(); ) {
                    ulx::u32 cp = __uii::font::decode_utf8(txt, i);
                    glyph_str = "";

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

                    obj.child(element(glyph_str)
                        .texture(glyph_str, font)
                        .size(ulx::size::autos())
                        .border(0).corner(0).back(ulx::color::empty())
                    );
                }

                return obj;
            }

            template<typename G>
                requires ulx::expect<G, ulx::geom>
            inline static constexpr auto unvisible(ulx::strvw id, G&& geom) -> element {
                return element(id)
                    .style({
                        .background = ulx::color::empty(),
                        .corner = 0, .borderw = 0
                    }).geom(std::forward<G>(geom));
            }

            inline static constexpr auto unbordered(ulx::strvw id) -> element {
                return element(id).corner(0).border(0);
            }
    };
}
