#pragma once

#include <types.hpp>

namespace ulx {
    class rect {
        public:
            enum size: ulx::i8 { 
                autosize = -1,
                fullsize = -2,
            };
    
        private:
            ulx::f32 x, y;
            ulx::f32 width, height;
    
        public:
            rect() = default;
            inline constexpr rect(ulx::f32 x, ulx::f32 y, ulx::f32 width, ulx::f32 height):
                x(x), y(y), width(width), height(height) {}
            bool operator==(const rect&) const = default;
    
        public:
            inline static auto new_pos(ulx::f32 x, ulx::f32 y) -> rect { return rect(x, y, 0, 0); }
            inline static auto new_size(ulx::f32 width, ulx::f32 height) -> rect { return rect(0, 0, width, height); }
            
        public:
            inline auto new_with_pos(ulx::f32 x, ulx::f32 y) const -> rect { return rect(x, y, width, height); }
            inline auto new_with_size(ulx::f32 width, ulx::f32 height) const -> rect { return rect(x, y, width, height); }
            inline auto new_with_sizeper(ulx::f32 width, ulx::f32 height) const -> rect { return rect(0, 0, get_width() * width, get_height() * height); }
            inline auto new_with_widthper(ulx::f32 width, ulx::f32 height_percent_of_width) const -> rect { return rect(0, 0, get_width() * width, get_width() * width * height_percent_of_width); }
            inline auto new_center(const rect& rec) const -> rect { return rect((get_width() - rec.get_width()) / 2, (get_height() - rec.get_height()) / 2, rec.get_width(), rec.get_height()); }
    
        public:
            inline auto get_x() const -> ulx::f32 { return x; }
            inline auto get_y() const -> ulx::f32 { return y; }
            inline auto get_width() const -> ulx::f32 { return width; }
            inline auto get_height() const -> ulx::f32 { return height; }
    };
}

template<> struct std::hash<ulx::rect> {
    size_t operator()(const ulx::rect& r) const noexcept {
        size_t h = hash<ulx::f32>{}(r.get_x());
        h ^= hash<ulx::f32>{}(r.get_y()) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= hash<ulx::f32>{}(r.get_width()) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= hash<ulx::f32>{}(r.get_height()) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }
};

template<> struct std::equal_to<ulx::rect> {
    bool operator()(const ulx::rect& a, const ulx::rect& b) const noexcept {
        return a.get_x() == b.get_x() && a.get_y() == b.get_y() && a.get_width() == b.get_width() && a.get_height() == b.get_height();
    }
};
