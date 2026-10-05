#pragma once

#include "types.hpp"

namespace ulx {
    class color {
        private:
            ulx::f32 r, g, b, a;
    
        public:
            color() = default;
            inline constexpr color(ulx::f32 red, ulx::f32 green, ulx::f32 blue, ulx::f32 alpha = 255):
                r(red), g(green), b(blue), a(alpha) {}
    
        public:
            constexpr auto to_rgba() const -> ulx::u32 { return (static_cast<ulx::u32>(r) << 24) | (static_cast<ulx::u32>(g) << 16) | (static_cast<ulx::u32>(b) << 8) | static_cast<ulx::u32>(a); }
            constexpr auto to_argb() const -> ulx::u32 { return (static_cast<ulx::u32>(a) << 24) | (static_cast<ulx::u32>(r) << 16) | (static_cast<ulx::u32>(g) << 8) | static_cast<ulx::u32>(b); }
            constexpr auto to_bgra() const -> ulx::u32 { return (static_cast<ulx::u32>(b) << 24) | (static_cast<ulx::u32>(g) << 16) | (static_cast<ulx::u32>(r) << 8) | static_cast<ulx::u32>(a); }
            constexpr auto to_abgr() const -> ulx::u32 { return (static_cast<ulx::u32>(a) << 24) | (static_cast<ulx::u32>(b) << 16) | (static_cast<ulx::u32>(g) << 8) | static_cast<ulx::u32>(r); }
            constexpr auto to_bgr() const -> ulx::u32 { return (static_cast<ulx::u32>(b) << 16) | (static_cast<ulx::u32>(g) << 8) | static_cast<ulx::u32>(r); }
            constexpr auto to_rgb() const -> ulx::u32 { return (static_cast<ulx::u32>(r) << 16) | (static_cast<ulx::u32>(g) << 8) | static_cast<ulx::u32>(b); }
    
            auto red() const -> ulx::f32 { return r; }
            auto green() const -> ulx::f32 { return g; }
            auto blue() const -> ulx::f32 { return b; }
            auto alpha() const -> ulx::f32 { return a; }
    
            inline constexpr auto operator==(const color& other) const -> bool = default;
            inline constexpr auto operator!=(const color& other) const -> bool = default;

        public:
            inline constexpr static auto empty() -> color { return color(0, 0, 0, 0); }
    };
}

template<> struct std::hash<ulx::color> {
    size_t operator()(const ulx::color& c) const noexcept {
        size_t h = 0;
        h ^= hash<ulx::f32>{}(c.red()) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= hash<ulx::f32>{}(c.green()) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= hash<ulx::f32>{}(c.blue()) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= hash<ulx::f32>{}(c.alpha()) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }
};

template<> struct std::equal_to<ulx::color> {
    bool operator()(const ulx::color& a, const ulx::color& b) const noexcept {
        return a == b;
    }
};
