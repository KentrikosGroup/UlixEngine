#pragma once
#include "types.hpp"
#include "size.hpp"

namespace ulx {
    class point {
        private:
            f32 m_x, m_y;
            
        public:
            inline constexpr point() = default;
            inline constexpr point(f32 x, f32 y): m_x(x), m_y(y) {}

            inline constexpr auto x() const -> f32 { return m_x; }
            inline constexpr auto y() const -> f32 { return m_y; }

        public:
            inline static constexpr auto center_to(const size& slf, const size& size) -> point {
                return { (size.w() - slf.w()) / 2.0f, (size.h() - slf.h()) / 2.0f };
            }
    };
}