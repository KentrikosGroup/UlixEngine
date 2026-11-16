#pragma once
#include <ulix/types.hpp>
#include <ulix/pixmap.hpp>
#include <ulix/point.hpp>

namespace ulx::ats {
    using uv = ulx::pack<ulx::point, ulx::size>;
    using idx = ulx::pack<ulx::u64, uv>;
    inline static constexpr auto push_pixmap(ulx::vec<ulx::pixmap>& pixmaps, ulx::u32& current, ulx::u32& current_x, ulx::u32& current_y, ulx::u32& max_height, ulx::pixmap& pixmap) -> idx {
        if (pixmap.w() > 256 || pixmap.h() > 256) {
            pixmaps.push_back(pixmap);
            return {pixmaps.size()-1, {{0, 0}, {1, 1}}};
        }
        
        ulx::pixmap& current_pixmap = pixmaps[current];
        
        ulx::u32 padded_w = pixmap.w() + 1;
        ulx::u32 padded_h = pixmap.h() + 1;
        
        if (current_x + padded_w > current_pixmap.w()) {
            current_y += max_height;
            current_x = 0;
            max_height = 0;
        } 
        if (current_y + padded_h > current_pixmap.h()) {
            current_x = current_y = max_height = 0;
            pixmaps.push_back(ulx::pixmap(2048, 2048));
            current = pixmaps.size()-1;
            return push_pixmap(pixmaps, current, current_x, current_y, max_height, pixmap);
        }
        
        ulx::u32 x = current_x;
        ulx::u32 y = current_y;
        
        if (padded_h > max_height) max_height = padded_h;
        current_x += padded_w;
        
        for (ulx::u32 row = 0; row < pixmap.h(); ++row) {
            std::memcpy(
                &current_pixmap.pixels()[((y + row) * current_pixmap.w() + x) * 4],
                &pixmap.pixels()[row * pixmap.w() * 4],
                pixmap.w() * 4
            );
        }
        
        ulx::f32 w = static_cast<ulx::f32>(current_pixmap.w());
        ulx::f32 h = static_cast<ulx::f32>(current_pixmap.h());
        return {current, {
            {x / w, y / h},
            {pixmap.w() / w, pixmap.h() / h}
        }};
    }
}