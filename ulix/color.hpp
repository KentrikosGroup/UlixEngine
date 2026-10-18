#pragma once
#include "types.hpp"

namespace ulx {
    class color {
        f32 r, g, b, a;
    
        public: inline constexpr color() = default;
        public: inline constexpr color(f32 red, f32 green, f32 blue, f32 alpha = 255): r(red), g(green), b(blue), a(alpha) {}
    
        public: inline constexpr auto to_rgba() const -> u32 { return (static_cast<u32>(r) << 24) | (static_cast<u32>(g) << 16) | (static_cast<u32>(b) << 8) | static_cast<u32>(a); }
        public: inline constexpr auto to_argb() const -> u32 { return (static_cast<u32>(a) << 24) | (static_cast<u32>(r) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(b); }
        public: inline constexpr auto to_bgra() const -> u32 { return (static_cast<u32>(b) << 24) | (static_cast<u32>(g) << 16) | (static_cast<u32>(r) << 8) | static_cast<u32>(a); }
        public: inline constexpr auto to_abgr() const -> u32 { return (static_cast<u32>(a) << 24) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r); }
        public: inline constexpr auto to_bgr() const -> u32 { return (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r); }
        public: inline constexpr auto to_rgb() const -> u32 { return (static_cast<u32>(r) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(b); }
    
        public: inline constexpr auto red() const -> f32 { return r; }
        public: inline constexpr auto green() const -> f32 { return g; }
        public: inline constexpr auto blue() const -> f32 { return b; }
        public: inline constexpr auto alpha() const -> f32 { return a; }
    
        public: inline constexpr static auto empty() -> color { return color(0, 0, 0, 0); }
            
        public: inline constexpr auto operator==(const color& other) const -> bool = default;
    };
}
