#pragma once

#include "color.hpp"
#include "types.hpp"

namespace ulx {
    class font {
        private:
            ulx::str font_name;
            ulx::u32 font_size;
            ulx::color font_color;
    
        public:
            inline font(
                const ulx::str& name = "arial.ttf", ulx::u32 size = 32,
                ulx::color color = ulx::color(255, 255, 255)
            ):
                font_name(name), font_size(size),
                font_color(color)
            {}
    
            inline constexpr auto operator==(const font& other) const -> bool = default;
    
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
    
        public:
            inline constexpr auto get_name() const -> const ulx::str& { return font_name; }
            inline constexpr auto get_size() const -> ulx::u32 { return font_size; }
            inline constexpr auto get_color() const -> const ulx::color& { return font_color; }

        public:
            inline constexpr auto make(ulx::u32 size = 32, const ulx::color& color = ulx::color(255, 255, 255)) const -> font {
                return font(font_name, size, color);
            }

        public:
            inline constexpr auto set_name(const ulx::str& name) -> font& {
                font_name = name;
                return *this;
            }

            inline constexpr auto set_size(ulx::u32 size) -> font& {
                font_size = size;
                return *this;
            }

            inline constexpr auto set_color(const ulx::color& color) -> font& {
                font_color = color;
                return *this;
            }
    };
}

template<> struct std::hash<ulx::font> {
    size_t operator()(const ulx::font& f) const noexcept {
        size_t h = hash<string>{}(f.get_name());
        h ^= hash<ulx::f32>{}(f.get_size()) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= hash<ulx::color>{}(f.get_color()) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }
};

template<> struct std::equal_to<ulx::font> {
    bool operator()(const ulx::font& a, const ulx::font& b) const noexcept {
        return a.get_name() == b.get_name() &&
               a.get_size() == b.get_size() &&
               a.get_color() == b.get_color();
    }
};
