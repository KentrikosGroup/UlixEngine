#pragma once
#include "types.hpp"

namespace ulx {
    class recsize {
        private:
            f32 width_, height_;
            
        public:
            inline constexpr recsize() = default;
            inline constexpr recsize(f32 width, f32 height): width_(width), height_(height) {}

            inline constexpr auto width() const -> f32 { return width_; }
            inline constexpr auto height() const -> f32 { return height_; }

            inline constexpr auto operator==(const recsize&) const -> bool = default;
            
        public:
            inline static constexpr auto from_ratio(f32 width, f32 ratio) -> recsize {
                return recsize(width, width * ratio);
            }

            inline static constexpr auto autow(f32 height) -> recsize { return {-1.0f, height}; }
            inline static constexpr auto autoh(f32 width) -> recsize { return {width, -1.0f}; }
            inline static constexpr auto autos() -> recsize { return {-1.0f, -1.0f}; }
            inline static constexpr auto fullw(f32 height) -> recsize { return {-2.0f, height}; }
            inline static constexpr auto fullh(f32 width) -> recsize { return {width, -2.0f}; }
            inline static constexpr auto fulls() -> recsize { return {-2.0f, -2.0f}; }
    };
}

template<> struct std::hash<ulx::recsize> {
    size_t operator()(const ulx::recsize& r) const noexcept {
        size_t h = hash<ulx::f32>{}(r.width());
        h ^= hash<ulx::f32>{}(r.height()) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }
};

template<> struct std::equal_to<ulx::recsize> {
    bool operator()(const ulx::recsize& a, const ulx::recsize& b) const {
        return a == b;
    }
};
