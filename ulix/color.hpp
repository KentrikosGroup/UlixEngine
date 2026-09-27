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
