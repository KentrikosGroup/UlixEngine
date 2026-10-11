#pragma once
#include "types.hpp"
#include "recsize.hpp"

namespace ulx {
    class recpos {
        private:
            f32 x_, y_;
            
        public:
            inline constexpr recpos() = default;
            inline constexpr recpos(f32 x, f32 y): x_(x), y_(y) {}

            inline constexpr auto x() const -> f32 { return x_; }
            inline constexpr auto y() const -> f32 { return y_; }

        public:
            inline static constexpr auto center_to(const recsize& slf, const recsize& size) -> recpos {
                return { (size.width() - slf.width()) / 2, (size.height() - slf.height()) / 2 };
            }
    };
}