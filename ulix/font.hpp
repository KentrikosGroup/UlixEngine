#pragma once
#include "color.hpp"
#include "types.hpp"
#include "stablestr.hpp"
#include "requires.hpp"
#include "ulix/autohasher.hpp"

namespace ulx {
    class font {
        pathstr font_name;
        f32 font_size;

        public: template<typename _pathstr = pathstr>
            requires ulx::expect<_pathstr, pathstr>
        inline font(pathstr&& name = "arial.ttf", f32 size = 16
        ): font_name(std::forward<_pathstr>(name)), font_size(size) {}

        public: inline constexpr auto operator==(const font& other) const -> bool {
            return font_name == other.font_name &&
            font_size == other.font_size;
        }

        public: template<typename _pathstr>
            requires ulx::expect<_pathstr, pathstr>
        inline auto name(_pathstr&& name) -> font& {
            font_name = std::forward<_pathstr>(name);
            return *this;
        }

        public: inline auto size(f32 size) -> font& {
            font_size = size;
            return *this;
        }

        public: inline constexpr auto get_name() const -> const pathstr& { return font_name; }
        public: inline constexpr auto get_size() const -> f32 { return font_size; }

        public: inline constexpr auto make(f32 size = 16) const -> font {
            return font(font_name.copy(), size);
        }
    };
}

namespace std {
    template<>
    struct hash<ulx::font> {
        size_t operator()(const ulx::font& f) const noexcept {
            return ulx::make_hash(f.get_name(), f.get_size());
        }
    };
}
