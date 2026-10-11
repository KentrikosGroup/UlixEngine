#pragma once

#include "font.hpp"
#include "__inside_impl/fontloader.hpp"
#include "file.hpp"
#include "log.hpp"
#include "recsize.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include <cassert>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <wincodec.h>
#include <winerror.h>
#include <wrl/client.h>
#include "__require_libraries/nanosvg/nanosvg.hpp"
#include "__require_libraries/stb_image/stb_image.hpp"

template<typename T> using aptr = Microsoft::WRL::ComPtr<T>;


namespace ulx {
    class pixmap {
        private:
            u32 texture_width;
            u32 texture_height;
            u64 texture_size;
            vec<u8> texture_pixels;
    
        public:
            pixmap() = default;
            pixmap(vec<u8> pixels, u32 width, u32 height) : texture_pixels(pixels), texture_width(width), texture_height(height), texture_size(pixels.size()) {}
    
            inline static auto from_bitmap(const file& image_file) -> pixmap {
                stablestr<260> file_path = image_file.path();
                i32 texture_width = 0, texture_height = 0, texture_channels = 0;
    
                u8* data = stbi_load(file_path.cstr(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
                if (!data) log::err("failed to load image (stb_image): {}", file_path, stbi_failure_reason());
                vec<u8> texture_pixels(data, data + texture_width * texture_height * 4);
                stbi_image_free(data);
    
                return pixmap(texture_pixels, texture_width, texture_height);
            }
    
            inline static auto from_vector(const file& image_file, u32 width, u32 height) -> pixmap {
                pathstr file_path = image_file.path();
                NSVGimage* svg = nsvgParseFromFile(file_path.cstr(), "px", 96.0f);
                if (!svg) log::err("failed to parse svg (nanosvg): {}", file_path);
                NSVGrasterizer* rast = nsvgCreateRasterizer();
    
                width *= 2; height *= 2;
                f32 scale = std::min(
                    static_cast<f32>(width) / svg->width,
                    static_cast<f32>(height) / svg->height
                );
                u32 stride = width * 4;
                vec<u8> texture_pixels(stride * height, 0);
    
                nsvgRasterize(rast, svg, 0, 0, scale, texture_pixels.data(), width, height, stride);
                nsvgDeleteRasterizer(rast);
                nsvgDelete(svg);
    
                return pixmap(texture_pixels, width, height);
            }
    
            inline static auto from_text(const str& text, const font f = font()) -> pixmap {
                const __uii::font::text_layout& text_layout = __uii::font::layout_text(text, f);
    
                u32 width = static_cast<u32>(text_layout.total_width);
                u32 height = static_cast<u32>(text_layout.total_height);
                if (width == 0 || height == 0) log::err("failed to rasterize text (stb_truetype): empty text bounds");
    
                bytes pixels(static_cast<u64>(width * height * 4));
                color color = f.get_color();
                u8 red = color.red(), green = color.green(), blue = color.blue();
                float alpha_scale = color.alpha() / 255.0f;
    
                for (const __uii::font::text_layout::glyph_position& glyph_position : text_layout.glyphs) {
                    const __uii::font::glyph_value& glyph_value = __uii::font::glyph_cache.find(glyph_position.key)->second;
    
                    for (i32 glyph_y = 0; glyph_y < glyph_position.height; ++glyph_y) {
                        for (i32 glyph_x = 0; glyph_x < glyph_position.width; ++glyph_x) {
                            i32 position_x = glyph_position.x + glyph_x;
                            i32 position_y = glyph_position.y + glyph_y;
                            if (static_cast<u32>(position_x) >= width || static_cast<u32>(position_y) >= height)
                                continue;
    
                            u8 alpha = static_cast<u8>(glyph_value.bitmap[static_cast<u64>(glyph_y * glyph_position.width + glyph_x)] * alpha_scale);
                            auto* dst = &pixels[static_cast<u64>((position_y * width + position_x) * 4)];
                            dst[0] = red; dst[1] = green; dst[2] = blue; dst[3] = alpha;
                        }
                    }
                }
    
                return pixmap(std::move(pixels), width, height);
            }
    
        public:
            inline constexpr auto size() const -> recsize { return recsize(texture_width, texture_height); }
            inline constexpr auto width() const -> u32 { return texture_width; }
            inline constexpr auto height() const -> u32 { return texture_height; }
            inline constexpr auto pixel_size() const -> u64 { return texture_size; }
            inline constexpr auto pixels() const -> const vec<ulx::u8> { return texture_pixels; }
    };
}