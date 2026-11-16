#pragma once
#include <ulix/types.hpp>
#include <ulix/stablestr.hpp>
#include <ulix/requires.hpp>
#include <ulix/autohasher.hpp>

namespace ulx {
    class font {
        pathstr font_path;
        f32 font_size;

        public: template<typename t_pathstr = pathstr>
            requires ulx::expect<t_pathstr, pathstr>
        inline font(t_pathstr&& path = "arial.ttf", f32 size = 16
        ): font_path(std::forward<t_pathstr>(path)), font_size(size) {}

        public: inline constexpr auto operator==(const font& other) const -> bool {
            return font_path == other.font_path &&
            font_size == other.font_size;
        }

        public: template<typename _pathstr>
            requires ulx::expect<_pathstr, pathstr>
        inline auto path(_pathstr&& name) -> font& {
            font_path = std::forward<_pathstr>(name);
            return *this;
        }

        public: inline auto size(f32 size) -> font& {
            font_size = size;
            return *this;
        }

        public: inline constexpr auto get_path() const -> const pathstr& { return font_path; }
        public: inline constexpr auto get_size() const -> f32 { return font_size; }

        public: inline constexpr auto make(f32 size = 16) const -> font {
            return font(font_path.copy(), size);
        }
    };
}

namespace std {
    template<>
    struct hash<ulx::font> {
        size_t operator()(const ulx::font& f) const noexcept {
            return ulx::make_hash(f.get_path(), f.get_size());
        }
    };
}
