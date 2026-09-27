#pragma once
#include "types.hpp"
#include "recsize.hpp"

namespace ulx {
    class recpos {
        private:
            ulx::f32 x_, y_;
            
        public:
            inline constexpr recpos() = default;
            inline constexpr recpos(ulx::f32 x, ulx::f32 y): x_(x), y_(y) {}

            inline constexpr auto x() const -> ulx::f32 { return x_; }
            inline constexpr auto y() const -> ulx::f32 { return y_; }

        public:
            inline static constexpr auto center_to(const ulx::recsize& slf, const ulx::recsize& size) -> ulx::recpos {
                return { (size.width() - slf.width()) / 2, (size.height() - slf.height()) / 2 };
            }
    };
}