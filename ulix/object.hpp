#pragma once

#include "callback.hpp"
#include "file.hpp"
#include "font.hpp"
#include "log.hpp"
#include "box.hpp"
#include "pixmap.hpp"
#include "recpos.hpp"
#include "recsize.hpp"
#include "color.hpp"
#include "align.hpp"
#include "requires.hpp"
#include "winfunc.hpp"
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
        ulx::f32 scale = 1.0f;
    };

    struct geom {
        ulx::recsize size = ulx::recsize(0, 0);
        ulx::recpos pos = ulx::recpos(0, 0);
    };

    struct layoutbox {
        ulx::box box = ulx::box::nonebox;
        ulx::u8 align = ulx::align::alignleft | ulx::align::aligntop;
        ulx::f32 padding = 0.0f;
    };

    class context;
    class object {
        public:
            using texture_variant = std::variant<
                std::monostate,                     // No texture
                ulx::file,                          // Bitmap
                ulx::pair<ulx::file, ulx::recsize>, // Vector
                ulx::pair<ulx::str, ulx::font>>;    // Text

            using mouse_event = ulx::callback<void(context&, object&)>;
            using released_event = mouse_event;
            using pressed_event = mouse_event;
            using hovered_event = mouse_event;
            using callback = ulx::callback<void(object&)>;

        private:
            ulx::color background_color = ulx::color(255, 255, 255, 255);
            ulx::color border_color = ulx::color(255, 255, 255, 255);
            ulx::f32 corner_radius = 16.0f;
            ulx::f32 border_width = 3.0f;
            ulx::recpos p = ulx::recpos(0, 0);
            ulx::recsize siz = ulx::recsize(200, 200);
            ulx::vec<ulx::object> objects;
            ulx::u8 alignment = ulx::align::center;
            ulx::f32 paddin = 5;
            ulx::box bx = ulx::box::nonebox;
            texture_variant texturev;
            ulx::f32 sc = 1.0f;
            ulx::strvw id_;
            released_event released_e;
            pressed_event pressed_e;
            hovered_event hovered_e;

        public:
            inline constexpr object() = default;

            template<typename S>
                requires ulx::expect<S, ulx::strvw>
            inline constexpr object(S&& id): id_(std::forward<S>(id)) {}

        public:
            template<typename S>
                requires ulx::expect<S, ulx::strvw>
            inline constexpr auto id(S&& id) -> object& {
                this->id_ = std::forward<S>(id);
                return *this;
            }

            inline constexpr auto style(const ulx::style& style) -> object& {
                background_color = style.background;
                border_color = style.border;
                corner_radius = style.corner;
                border_width = style.borderw;
                sc = style.scale;
                return *this;
            }

            inline constexpr auto back(const ulx::color& color) -> object& {
                background_color = color;
                return *this;
            }

            inline constexpr auto border(const ulx::color& color) -> object& {
                border_color = color;
                return *this;
            }

            inline constexpr auto corner(ulx::f32 radius) -> object& {
                corner_radius = radius;
                return *this;
            }

            inline constexpr auto border(ulx::f32 width) -> object& {
                border_width = width;
                return *this;
            }

            inline constexpr auto size(const ulx::recsize& siz) -> object& {
                this->siz = siz;
                return *this;
            }

            inline constexpr auto pos(const ulx::recpos& p) -> object& {
                this->p = ulx::recpos(p.x(), p.y());
                return *this;
            }

            inline constexpr auto child(const object& obj) -> object& {
                objects.push_back(obj);
                return *this;
            }

            inline constexpr auto texture(const ulx::file& bitmap_or_svg) -> object& {
                if (!bitmap_or_svg.exists()) ulx::log::err("{} file {} doesn't exists", bitmap_or_svg.path());
                texturev = bitmap_or_svg;
                return *this;
            }

            inline constexpr auto texture(const ulx::file& svg, const ulx::recsize& size) -> object& {
                if (!svg.exists()) ulx::log::err("SVG file {} doesn't exists", svg.path());
                texturev = std::make_pair(svg, size);
                return *this;
            }

            inline constexpr auto texture(const ulx::str& text, const ulx::font& font = ulx::font()) -> object& {
                texturev = std::make_pair(text, font);
                return *this;
            }

            inline constexpr auto align(ulx::u8 alignment) -> object& {
                this->alignment = alignment;
                return *this;
            }

            inline constexpr auto pad(ulx::f32 padding) -> object& {
                paddin = padding;
                return *this;
            }

            inline constexpr auto box(ulx::box bx) -> object& {
                this->bx = bx;
                return *this;
            }

            inline constexpr auto scale(ulx::f32 sc) -> object& {
                this->sc = sc;
                return *this;
            }

            template<typename R>
                requires ulx::expect<R, released_event>
            inline constexpr auto released(R&& event) -> object& {
                released_e = std::forward<R>(event);
                return *this;
            }

            template<typename P>
                requires ulx::expect<P, pressed_event>
            inline constexpr auto pressed(P&& event) -> object& {
                pressed_e = std::forward<P>(event);
                return *this;
            }

            template<typename H>
                requires ulx::expect<H, hovered_event>
            inline constexpr auto hovered(H&& event) -> object& {
                hovered_e = std::forward<H>(event);
                return *this;
            }

            inline constexpr auto geom(const geom& geom) -> object& {
                p = ulx::recpos(geom.pos.x(), geom.pos.y() + 0.5);
                siz = geom.size;
                return *this;
            }

            inline constexpr auto layout(const layoutbox& layout) -> object& {
                bx = layout.box;
                paddin = layout.padding;
                alignment = layout.align;
                return *this;
            }

            template<typename C>
                requires ulx::expect<C, callback>
            inline constexpr auto call(C&& fn) -> object& {
                std::forward<C>(fn)(*this);
                return *this;
            }

            template<typename C>
                requires ulx::expect<C, callback>
            inline constexpr auto when(bool cond, C&& fn) -> object& {
                if (cond)
                    std::forward<C>(fn)(*this);
                return *this;
            }

            template<typename C, typename CE>
                requires ulx::expect<C, callback> &&
                         ulx::expect<CE, callback>
            inline constexpr auto when(bool cond, C&& fn, CE&& else_fn) -> object& {
                if (cond)
                    std::forward<C>(fn)(*this);
                else
                    std::forward<CE>(else_fn)(*this);
                return *this;
            }

        public:
            inline constexpr auto get_background_color() const -> const ulx::color& { return background_color; }
            inline constexpr auto get_border_color() const -> const ulx::color& { return border_color; }
            inline constexpr auto get_corner_radius() const -> ulx::f32 { return corner_radius; }
            inline constexpr auto get_border_width() const -> ulx::f32 { return border_width; }
            inline constexpr auto get_size() const -> const ulx::recsize& { return siz; }
            inline constexpr auto get_pos() const -> const ulx::recpos& { return p; }
            inline constexpr auto get_objects() const -> const ulx::vec<ulx::object>& { return objects; }
            inline constexpr auto get_texture() const -> const texture_variant& { return texturev; }
            inline constexpr auto get_alignment() const -> ulx::u8 { return alignment; }
            inline constexpr auto get_padding() const -> ulx::f32 { return paddin; }
            inline constexpr auto get_layout() const -> ulx::box { return bx; }
            inline constexpr auto get_scale() const -> ulx::f32 { return sc; }
            inline constexpr auto get_id() const -> const ulx::strvw& { return id_; }
            inline constexpr auto get_released_event() const -> const released_event& { return released_e; }
            inline constexpr auto get_pressed_event() const -> const pressed_event& { return pressed_e; }
            inline constexpr auto get_hovered_event() const -> const hovered_event& { return hovered_e; }

        public:
            inline static constexpr auto box() -> object {
                return object().size(ulx::recsize(0, 0));
            }

            template<typename F>
                requires ulx::expect<F, ulx::font>
            inline static constexpr auto text(const ulx::str& txt, F&& font, ulx::align align = ulx::align::alignbottom) -> object {
                ulx::i32 max_h = __uii::font::max_height_of(font);
                object obj = box().pad(0)
                    .align(ulx::alignleft | align)
                    .box(ulx::horbox).size(ulx::recsize(0, max_h));

                ulx::str glyph_str;
                for (ulx::size i = 0; i < txt.size(); ) {
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

                    obj.child(object(glyph_str)
                        .texture(glyph_str, font)
                        .size(ulx::recsize::autos())
                        .border(0).corner(0).back(ulx::color::empty())
                    );
                }

                return obj;
            }

            inline static constexpr auto text(const ulx::str& txt, ulx::align align = ulx::align::alignbottom) -> object {
                return text(txt, ulx::font(), align);
            }

            template<typename G>
                requires ulx::expect<G, ulx::geom>
            inline static constexpr auto unvisible(ulx::strvw id, G&& geom) -> object {
                return object(id)
                    .style({
                        .background = ulx::color::empty(),
                        .corner = 0, .borderw = 0
                    }).geom(std::forward<G>(geom));
            }

            inline static constexpr auto unbordered(ulx::strvw id) -> object {
                return object(id).corner(0).border(0);
            }
    };
}
