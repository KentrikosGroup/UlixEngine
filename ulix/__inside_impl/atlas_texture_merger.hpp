#pragma once
#include "../types.hpp"
#include "../pixmap.hpp"
#include "../point.hpp"

namespace __uii::ats {
    using uv = ulx::pack<ulx::point, ulx::size>;
    using idx = ulx::pack<ulx::u32, uv>;
    inline static constexpr auto push_pixmap(ulx::vec<ulx::pixmap>& pixmaps, ulx::u32& current, ulx::u32& current_x, ulx::u32& current_y, ulx::u32& max_height, ulx::pixmap& pixmap) -> idx {
        if (pixmap.width() > 2048 || pixmap.height() > 2048) {
            pixmaps.push_back(pixmap);
            return {static_cast<ulx::u32>(pixmaps.size()-1), {{0, 0}, {1, 1}}};
        }
        
        ulx::pixmap& current_pixmap = pixmaps[current];
        if (current_x + pixmap.width() > current_pixmap.width()) {
            current_y += max_height;
            current_x = 0;
            max_height = 0;
        } if (current_y + pixmap.height() > current_pixmap.height()) {
            ++current;
            current_x = current_y = max_height = 0;
            pixmaps.push_back(ulx::pixmap(2048, 2048));
            return push_pixmap(pixmaps, current, current_x, current_y, max_height, pixmap);
        }
        
        ulx::u32 x = current_x;
        ulx::u32 y = current_y;
        
        if (pixmap.height() > max_height) max_height = pixmap.height();
        current_x += pixmap.width();
        
        for (ulx::u32 row = 0; row < pixmap.height(); ++row) {
            std::memcpy(
                &current_pixmap.pixels()[((y + row) * current_pixmap.width() + x) * 4],
                &pixmap.pixels()[row * pixmap.width() * 4],
                pixmap.width() * 4
            );
        }
    
        ulx::f32 w = static_cast<ulx::f32>(current_pixmap.width());
        ulx::f32 h = static_cast<ulx::f32>(current_pixmap.height());
        return {current, {
            {x / w, y / h},
            {pixmap.width() / w, pixmap.height() / h}
        }};
    }
}