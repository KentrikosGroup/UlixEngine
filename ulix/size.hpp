#pragma once
#include "types.hpp"
#include "ulix/autohasher.hpp"

namespace ulx {
    class size {
        f32 m_width, m_height;
            
        public: inline constexpr size() = default;
        public: inline constexpr size(f32 width, f32 height): m_width(width), m_height(height) {}

        public: inline constexpr auto width() const -> f32 { return m_width; }
        public: inline constexpr auto height() const -> f32 { return m_height; }

        public: inline constexpr auto operator==(const size&) const -> bool = default;
            
        public: inline static constexpr auto from_ratio(f32 width, f32 ratio) -> size {
            return size(width, width * ratio);
        }

        public: inline static constexpr auto autow(f32 height) -> size { return {-1.0f, height}; }
        public: inline static constexpr auto autoh(f32 width) -> size { return {width, -1.0f}; }
        public: inline static constexpr auto autos() -> size { return {-1.0f, -1.0f}; }
        public: inline static constexpr auto fullw(f32 height) -> size { return {-2.0f, height}; }
        public: inline static constexpr auto fullh(f32 width) -> size { return {width, -2.0f}; }
        public: inline static constexpr auto fulls() -> size { return {-2.0f, -2.0f}; }
    };
}

namespace std {
    template<>
    struct hash<ulx::size> {
        size_t operator()(const ulx::size& s) const noexcept {
            return ulx::make_hash(s.width(), s.height());
        }
    };
}
