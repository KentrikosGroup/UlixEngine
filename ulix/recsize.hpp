#pragma once
#include "types.hpp"

namespace ulx {
    class recsize {
        private:
            ulx::f32 width_, height_;
            
        public:
            inline constexpr recsize() = default;
            inline constexpr recsize(ulx::f32 width, ulx::f32 height): width_(width), height_(height) {}

            inline constexpr auto width() const -> ulx::f32 { return width_; }
            inline constexpr auto height() const -> ulx::f32 { return height_; }

            inline constexpr auto operator==(const recsize&) const -> bool = default;
            
        public:
            inline static constexpr auto from_ratio(ulx::f32 width, ulx::f32 ratio) -> recsize {
                return recsize(width, width * ratio);
            }

            inline static constexpr auto autow(ulx::f32 height) -> recsize { return recsize(-1.0f, height); }
            inline static constexpr auto autoh(ulx::f32 width) -> recsize { return recsize(width, -1.0f); }
            inline static constexpr auto autos() -> recsize { return recsize(-1.0f, -1.0f); }
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
