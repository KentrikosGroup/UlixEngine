#pragma once

#include "file.hpp"
#include "font.hpp"
#include "log.hpp"
#include "layout.hpp"
#include "pixmap.hpp"
#include "anchor.hpp"
#include "rect.hpp"
#include "color.hpp"
#include "align.hpp"
#include <variant>

namespace ulx {
    class object {
        public:
            enum class TextureType {
                Bitmap, Vector,
                Text, None
            };
    
            using PixmapType = std::variant<
                ulx::file,                     // Bitmap
                std::pair<ulx::file, ulx::rect>,   // Vector
                std::pair<ulx::str, ulx::font> // Text
            >;
    
        private:
            ulx::color top_left_color = ulx::color(255, 255, 255, 255);
            ulx::color top_right_color = ulx::color(255, 255, 255, 255);
            ulx::color bottom_left_color = ulx::color(255, 255, 255, 255);
            ulx::color bottom_right_color = ulx::color(255, 255, 255, 255);
            ulx::color top_left_border_color = ulx::color(255, 255, 255, 255);
            ulx::color top_right_border_color = ulx::color(255, 255, 255, 255);
            ulx::color bottom_left_border_color = ulx::color(255, 255, 255, 255);
            ulx::color bottom_right_border_color = ulx::color(255, 255, 255, 255);
            ulx::f32 top_left_corner_radius = 16.0f;
            ulx::f32 top_right_corner_radius = 16.0f;
            ulx::f32 bottom_left_corner_radius = 16.0f;
            ulx::f32 bottom_right_corner_radius = 16.0f;
            ulx::f32 top_left_border_width = 1.0f;
            ulx::f32 top_right_border_width = 1.0f;
            ulx::f32 bottom_left_border_width = 1.0f;
            ulx::f32 bottom_right_border_width = 1.0f;
            ulx::rect rec = ulx::rect(100, 100, 200, 200);
            ulx::vec<ulx::object> objects;
            ulx::u8 alignment = ulx::align::Center;
            ulx::f32 vpaddin = 5;
            ulx::f32 hpaddin = 5;
            ulx::layout objlayout = ulx::layout::none;
            bool in_parent_layout = false;
            PixmapType texture_variant;
            TextureType texture_type = TextureType::None;
            ulx::f32 sc = 1.0f;
    
        public:
            object() = default;
            object([[maybe_unused]] const ulx::str& id) {}
    
        public:
            inline auto fill(const ulx::anchor anchor, const ulx::color& color) -> object&& {
                switch (anchor) {
                    case ulx::anchor::topleft: top_left_color = color; break;
                    case ulx::anchor::topright: top_right_color = color; break;
                    case ulx::anchor::bottomleft: bottom_left_color = color; break;
                    case ulx::anchor::bottomright: bottom_right_color = color; break;
                    case ulx::anchor::all:
                        top_left_color = color;
                        top_right_color = color;
                        bottom_left_color = color;
                        bottom_right_color = color;
                        break;
                }
    
                return std::move(*this);
            }
    
            inline auto border(const ulx::anchor anchor, const ulx::color& color) -> object&& {
                switch (anchor) {
                    case ulx::anchor::topleft: top_left_border_color = color; break;
                    case ulx::anchor::topright: top_right_border_color = color; break;
                    case ulx::anchor::bottomleft: bottom_left_border_color = color; break;
                    case ulx::anchor::bottomright: bottom_right_border_color = color; break;
                    case ulx::anchor::all:
                        top_left_border_color = color;
                        top_right_border_color = color;
                        bottom_left_border_color = color;
                        bottom_right_border_color = color;
                        break;
                }
    
                return std::move(*this);
            }
    
            inline auto corner(const ulx::anchor anchor, ulx::f32 radius) -> object&& {
                switch (anchor) {
                    case ulx::anchor::topleft: top_left_corner_radius = radius; break;
                    case ulx::anchor::topright: top_right_corner_radius = radius; break;
                    case ulx::anchor::bottomleft: bottom_left_corner_radius = radius; break;
                    case ulx::anchor::bottomright: bottom_right_corner_radius = radius; break;
                    case ulx::anchor::all:
                        top_left_corner_radius = radius;
                        top_right_corner_radius = radius;
                        bottom_left_corner_radius = radius;
                        bottom_right_corner_radius = radius;
                        break;
                }
    
                return std::move(*this);
            }
    
            inline auto border(const ulx::anchor anchor, ulx::f32 width) -> object&& {
                switch (anchor) {
                    case ulx::anchor::topleft: top_left_border_width = width; break;
                    case ulx::anchor::topright: top_right_border_width = width; break;
                    case ulx::anchor::bottomleft: bottom_left_border_width = width; break;
                    case ulx::anchor::bottomright: bottom_right_border_width = width; break;
                    case ulx::anchor::all:
                        top_left_border_width = width;
                        top_right_border_width = width;
                        bottom_left_border_width = width;
                        bottom_right_border_width = width;
                        break;
                }
    
                return std::move(*this);
            }
    
            inline auto rect(const ulx::rect& rec) -> object&& {
                this->rec = rec;
    
                return std::move(*this);
            }
    
            inline auto child(object&& obj) -> object&& {
                objects.push_back(obj);
    
                return std::move(*this);
            }
    
            inline auto texture(const ulx::file& bitmap) -> object&& {
                if (!bitmap.exists()) ulx::log::ulixerr("Bitmap file {} doesn't exists", bitmap.get_file_path());
                texture_type = TextureType::Bitmap;
                texture_variant = bitmap;
    
                return std::move(*this);
            }
    
            inline auto texture(const ulx::file& svg, const ulx::rect& rect) -> object&& {
                if (!svg.exists()) ulx::log::ulixerr("SVG file {} doesn't exists", svg.get_file_path());
                texture_type = TextureType::Vector;
                texture_variant = std::make_pair(svg, rect);
    
                return std::move(*this);
            }
    
            inline auto texture(const ulx::str& text, const ulx::font& font = ulx::font()) -> object&& {
                texture_variant = std::make_pair(text, font);
                texture_type = TextureType::Text;
    
                return std::move(*this);
            }
    
            inline auto align(ulx::u8 alignment) -> object&& {
                this->alignment = alignment;
                return std::move(*this);
            }
    
            inline auto vpadding(ulx::f32 padding) -> object&& {
                vpaddin = padding;
                return std::move(*this);
            }
    
            inline auto hpadding(ulx::f32 padding) -> object&& {
                hpaddin = padding;
                return std::move(*this);
            }
    
            inline auto layout(ulx::layout objlayout) -> object&& {
                this->objlayout = objlayout;
                return std::move(*this);
            }
    
            inline auto inlayout(bool in) -> object&& {
                this->in_parent_layout = in;
                return std::move(*this);
            }
    
            inline auto scale(ulx::f32 sc) -> object&& {
                this->sc = sc;
                return std::move(*this);
            }
    
        public:
            inline auto get_top_left_color() const -> const ulx::color& { return top_left_color; }
            inline auto get_top_right_color() const -> const ulx::color& { return top_right_color; }
            inline auto get_bottom_left_color() const -> const ulx::color& { return bottom_left_color; }
            inline auto get_bottom_right_color() const -> const ulx::color& { return bottom_right_color; }
    
            inline auto get_top_left_border_color() const -> const ulx::color& { return top_left_border_color; }
            inline auto get_top_right_border_color() const -> const ulx::color& { return top_right_border_color; }
            inline auto get_bottom_left_border_color() const -> const ulx::color& { return bottom_left_border_color; }
            inline auto get_bottom_right_border_color() const -> const ulx::color& { return bottom_right_border_color; }
    
            inline auto get_top_left_corner_radius() const -> ulx::f32 { return top_left_corner_radius; }
            inline auto get_top_right_corner_radius() const -> ulx::f32 { return top_right_corner_radius; }
            inline auto get_bottom_left_corner_radius() const -> ulx::f32 { return bottom_left_corner_radius; }
            inline auto get_bottom_right_corner_radius() const -> ulx::f32 { return bottom_right_corner_radius; }
    
            inline auto get_top_left_border_width() const -> ulx::f32 { return top_left_border_width; }
            inline auto get_top_right_border_width() const -> ulx::f32 { return top_right_border_width; }
            inline auto get_bottom_left_border_width() const -> ulx::f32 { return bottom_left_border_width; }
            inline auto get_bottom_right_border_width() const -> ulx::f32 { return bottom_right_border_width; }
    
            inline auto get_rect() const -> const ulx::rect& { return rec; }
            inline auto get_objects() const -> const ulx::vec<ulx::object>& { return objects; }
            inline auto get_texture() const -> const PixmapType& { return texture_variant; }
            inline auto get_texture_type() const -> TextureType { return texture_type; }
            inline auto get_alignment() const -> ulx::u8 { return alignment; }
            inline auto get_vpadding() const -> ulx::f32 { return vpaddin; }
            inline auto get_hpadding() const -> ulx::f32 { return hpaddin; }
            inline auto get_layout() const -> ulx::layout { return objlayout; }
            inline auto get_in_parent_layout() const -> bool { return in_parent_layout; }
            inline auto get_scale() const -> ulx::f32 { return sc; }
    };
}