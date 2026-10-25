#pragma once
#include "color.hpp"
#include "types.hpp"
#include "stablestr.hpp"
#include "requires.hpp"

namespace ulx {
    class font {
        pathstr font_name;
        u32 font_size;
        color font_color;
    
        public: template<typename _pathstr = pathstr> 
            requires ulx::expect<_pathstr, pathstr>
        inline font(pathstr&& name = "arial.ttf", u32 size = 16, color color = {255, 255, 255}
        ): font_name(std::forward<_pathstr>(name)), font_size(size), font_color(color) {}
    
        public: inline constexpr auto operator==(const font&) const -> bool = default;

        public: template<typename _pathstr>
            requires ulx::expect<_pathstr, pathstr>
        inline auto name(_pathstr&& name) -> font& {
            font_name = std::forward<_pathstr>(name);
            return *this;
        }

        public: inline auto size(u32 size) -> font& {
            font_size = size;
            return *this;
        }

        public: template<typename _color>
            requires ulx::expect<_color, color>
        inline auto color(_color&& color) -> font& {
            font_color = std::forward<_color>(color);
            return *this;
        }

        public: inline constexpr auto get_name() const -> const pathstr& { return font_name; }
        public: inline constexpr auto get_size() const -> u32 { return font_size; }
        public: inline constexpr auto get_color() const -> const ulx::color& { return font_color; }

        public: inline constexpr auto make(u32 size = 16, const ulx::color& color = ulx::color(255, 255, 255)) const -> font {
            return font(font_name.copy(), size, color);
        }
    };
}
