#pragma once

#include "file.hpp"
#include "font.hpp"
#include "log.hpp"
#include "layout.hpp"
#include "pixmap.hpp"
#include "recpos.hpp"
#include "recsize.hpp"
#include "rect.hpp"
#include "color.hpp"
#include "align.hpp"
#include "requires.hpp"
#include <variant>

namespace ulx {
    struct style {
        ulx::color background = ulx::color(255, 255, 255, 255);
        ulx::color border = ulx::color(255, 255, 255, 255);
        ulx::f32 corner = 16.0f;
        ulx::f32 borderw = 16.0f;
        ulx::f32 scale = 1.0f;
    };
    
    class object {
        public:
            using texture_variant = std::variant<
                std::monostate,                    // No texture
                ulx::file,                         // Bitmap
                ulx::pair<ulx::file, ulx::recsize>,   // Vector
                ulx::pair<ulx::str, ulx::font>>;   // Text

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
            ulx::layout layout_ = ulx::layout::nonebox;
            texture_variant texturev;
            ulx::f32 sc = 1.0f;
            ulx::strvw id;

        public:
            inline constexpr object() = default;

            template<typename S>
                requires ulx::expect<S, ulx::strvw>
            inline constexpr object(S&& id): id(std::forward<S>(id)) {}

        public:
            inline constexpr auto style(const ulx::style& style) -> object& {
                background_color = style.background;
                border_color = style.border;
                corner_radius = style.corner;
                border_width = style.borderw;
                sc = style.scale;
                return *this;
            }
            
            inline constexpr auto background(const ulx::color& color) -> object& {
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
                this->p = p;
                return *this;
            }

            inline constexpr auto child(const object& obj) -> object& {
                objects.push_back(obj);
                return *this;
            }

            inline constexpr auto texture(const ulx::file& bitmap_or_svg) -> object& {
                if (!bitmap_or_svg.exists()) ulx::log::err("{} file {} doesn't exists", bitmap_or_svg.get_file_path());
                texturev = bitmap_or_svg;
                return *this;
            }

            inline constexpr auto texture(const ulx::file& svg, const ulx::recsize& size) -> object& {
                if (!svg.exists()) ulx::log::err("SVG file {} doesn't exists", svg.get_file_path());
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

            inline constexpr auto padding(ulx::f32 padding) -> object& {
                paddin = padding;
                return *this;
            }

            inline constexpr auto layout(ulx::layout layout_) -> object& {
                this->layout_ = layout_;
                return *this;
            }

            inline constexpr auto scale(ulx::f32 sc) -> object& {
                this->sc = sc;
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
            inline constexpr auto get_layout() const -> ulx::layout { return layout_; }
            inline constexpr auto get_scale() const -> ulx::f32 { return sc; }
            inline constexpr auto get_id() const -> const ulx::strvw& { return id; }

        public:
            inline static constexpr auto box() -> object {
                return object().size(ulx::recsize(0, 0));
            }
    };
}
