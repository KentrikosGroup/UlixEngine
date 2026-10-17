#pragma once
#include "types.hpp"

namespace ulx {
    class size {
        private:
            f32 width_, height_;
            
        public:
            inline constexpr size() = default;
            inline constexpr size(f32 width, f32 height): width_(width), height_(height) {}

            inline constexpr auto width() const -> f32 { return width_; }
            inline constexpr auto height() const -> f32 { return height_; }

            inline constexpr auto operator==(const size&) const -> bool = default;
            
        public:
            inline static constexpr auto from_ratio(f32 width, f32 ratio) -> size {
                return size(width, width * ratio);
            }

            inline static constexpr auto autow(f32 height) -> size { return {-1.0f, height}; }
            inline static constexpr auto autoh(f32 width) -> size { return {width, -1.0f}; }
            inline static constexpr auto autos() -> size { return {-1.0f, -1.0f}; }
            inline static constexpr auto fullw(f32 height) -> size { return {-2.0f, height}; }
            inline static constexpr auto fullh(f32 width) -> size { return {width, -2.0f}; }
            inline static constexpr auto fulls() -> size { return {-2.0f, -2.0f}; }
    };
}
