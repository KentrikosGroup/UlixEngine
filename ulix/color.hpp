#pragma once

#include <types.hpp>

namespace ulx {
    class color {
        private:
            ulx::f32 red, green, blue, alpha;
    
        public:
            color() = default;
            inline constexpr color(ulx::f32 red, ulx::f32 green, ulx::f32 blue, ulx::f32 alpha = 255):
                red(red), green(green), blue(blue), alpha(alpha) {}
    
        public:
            constexpr auto to_rgba() const -> ulx::u32 { return (static_cast<ulx::u32>(red) << 24) | (static_cast<ulx::u32>(green) << 16) | (static_cast<ulx::u32>(blue) << 8) | static_cast<ulx::u32>(alpha); }
            constexpr auto to_argb() const -> ulx::u32 { return (static_cast<ulx::u32>(alpha) << 24) | (static_cast<ulx::u32>(red) << 16) | (static_cast<ulx::u32>(green) << 8) | static_cast<ulx::u32>(blue); }
            constexpr auto to_bgra() const -> ulx::u32 { return (static_cast<ulx::u32>(blue) << 24) | (static_cast<ulx::u32>(green) << 16) | (static_cast<ulx::u32>(red) << 8) | static_cast<ulx::u32>(alpha); }
            constexpr auto to_abgr() const -> ulx::u32 { return (static_cast<ulx::u32>(alpha) << 24) | (static_cast<ulx::u32>(blue) << 16) | (static_cast<ulx::u32>(green) << 8) | static_cast<ulx::u32>(red); }
            constexpr auto to_bgr() const -> ulx::u32 { return (static_cast<ulx::u32>(blue) << 16) | (static_cast<ulx::u32>(green) << 8) | static_cast<ulx::u32>(red); }
            constexpr auto to_rgb() const -> ulx::u32 { return (static_cast<ulx::u32>(red) << 16) | (static_cast<ulx::u32>(green) << 8) | static_cast<ulx::u32>(blue); }
    
            auto get_red() const -> ulx::f32 { return red; }
            auto get_green() const -> ulx::f32 { return green; }
            auto get_blue() const -> ulx::f32 { return blue; }
            auto get_alpha() const -> ulx::f32 { return alpha; }
    
            inline constexpr auto operator==(const color& other) const -> bool {
                return red == other.red && green == other.green && blue == other.blue && alpha == other.alpha;
            }
    
            inline constexpr auto operator!=(const color& other) const -> bool {
                return !(*this == other);
            }
    };
}
