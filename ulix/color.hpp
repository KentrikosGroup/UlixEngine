#pragma once
#include "types.hpp"

namespace ulx {
    class color {
        private:
            f32 r, g, b, a;
    
        public:
            color() = default;
            inline constexpr color(f32 red, f32 green, f32 blue, f32 alpha = 255):
                r(red), g(green), b(blue), a(alpha) {}
    
        public:
            inline constexpr auto to_rgba() const -> u32 { return (static_cast<u32>(r) << 24) | (static_cast<u32>(g) << 16) | (static_cast<u32>(b) << 8) | static_cast<u32>(a); }
            inline constexpr auto to_argb() const -> u32 { return (static_cast<u32>(a) << 24) | (static_cast<u32>(r) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(b); }
            inline constexpr auto to_bgra() const -> u32 { return (static_cast<u32>(b) << 24) | (static_cast<u32>(g) << 16) | (static_cast<u32>(r) << 8) | static_cast<u32>(a); }
            inline constexpr auto to_abgr() const -> u32 { return (static_cast<u32>(a) << 24) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r); }
            inline constexpr auto to_bgr() const -> u32 { return (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r); }
            inline constexpr auto to_rgb() const -> u32 { return (static_cast<u32>(r) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(b); }
    
            inline constexpr auto red() const -> f32 { return r; }
            inline constexpr auto green() const -> f32 { return g; }
            inline constexpr auto blue() const -> f32 { return b; }
            inline constexpr auto alpha() const -> f32 { return a; }
    
            inline constexpr auto operator==(const color& other) const -> bool = default;

        public:
            inline constexpr static auto empty() -> color { return color(0, 0, 0, 0); }
    };
}
