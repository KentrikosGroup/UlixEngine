#pragma once
#include "types.hpp"
#include "size.hpp"

namespace ulx {
    class point {
        private:
            f32 x_, y_;
            
        public:
            inline constexpr point() = default;
            inline constexpr point(f32 x, f32 y): x_(x), y_(y) {}

            inline constexpr auto x() const -> f32 { return x_; }
            inline constexpr auto y() const -> f32 { return y_; }

        public:
            inline static constexpr auto center_to(const size& slf, const size& size) -> point {
                return { (size.width() - slf.width()) / 2, (size.height() - slf.height()) / 2 };
            }
    };
}