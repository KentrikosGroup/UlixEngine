#pragma once

#include <color.hpp>
#include <types.hpp>

namespace ulx {
    class font {
        private:
            ulx::str font_name;
            ulx::u32 font_size;
            ulx::color font_color;
            ulx::f32 sc;
    
        public:
            inline font(
                const ulx::str& name = "arial.ttf", ulx::u32 size = 12,
                ulx::color color = ulx::color(255, 255, 255), ulx::f32 scale = 2
            ):
                font_name(name), font_size(size),
                font_color(color), sc(scale)
            {}
    
            bool operator==(const font& other) const {
                return font_name == other.font_name && font_size == other.font_size && font_color == other.font_color && sc == other.sc;
            };
    
            inline auto name(const ulx::str& name) -> font& {
                font_name = name;
                return *this;
            }
    
            inline auto size(ulx::u32 size) -> font& {
                font_size = size;
                return *this;
            }
    
            inline auto color(const ulx::color& color) -> font& {
                font_color = color;
                return *this;
            }
    
            inline auto scale(ulx::f32 sc) -> font& {
                this->sc = sc;
                return *this;
            }
    
        public:
            inline auto get_name() const -> ulx::str { return font_name; }
            inline auto get_size() const -> ulx::u32 { return font_size; }
            inline auto get_color() const -> const ulx::color& { return font_color; }
            inline auto get_scale() const -> ulx::f32 { return sc; }
    };
}

template<> struct std::hash<ulx::font> {
    size_t operator()(const ulx::font& f) const noexcept {
        size_t h = hash<string>{}(f.get_name());
        h ^= hash<ulx::f32>{}(f.get_size()) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= hash<ulx::f32>{}(f.get_scale()) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }
};

template<> struct std::equal_to<ulx::font> {
    bool operator()(const ulx::font& a, const ulx::font& b) const noexcept {
        return a.get_name() == b.get_name() &&
               a.get_size() == b.get_size() &&
               a.get_color() == b.get_color() &&
               a.get_scale() == b.get_scale();
    }
};
