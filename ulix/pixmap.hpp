#pragma once

#include "font.hpp"
#include "__inside_impl/fontloader.hpp"
#include "file.hpp"
#include "log.hpp"
#include "rect.hpp"
#include "types.hpp"
#include <cassert>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <wincodec.h>
#include <winerror.h>
#include <wrl/client.h>
#include "__require_libraries/nanosvg/nanosvg.hpp"
#include "__require_libraries/stb_image/stb_image.hpp"

template<typename T> using Pointer = Microsoft::WRL::ComPtr<T>;


namespace ulx {
    class pixmap {
        private:
            ulx::u32 texture_width;
            ulx::u32 texture_height;
            ulx::u64 texture_size;
            ulx::vec<ulx::u8> texture_pixels;
    
        public:
            pixmap() = default;
            pixmap(ulx::vec<ulx::u8> pixels, ulx::u32 width, ulx::u32 height) : texture_pixels(pixels), texture_width(width), texture_height(height), texture_size(pixels.size()) {}
    
            inline static auto from_bitmap(const ulx::file& image_file) -> pixmap {
                ulx::str file_path = image_file.get_file_path();
                ulx::i32 texture_width = 0, texture_height = 0, texture_channels = 0;
    
                ulx::u8* data = stbi_load(file_path.c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
                if (!data) ulx::log::ulixerr("failed to load image (stb_image): {}", file_path, stbi_failure_reason());
                ulx::vec<ulx::u8> texture_pixels(data, data + texture_width * texture_height * 4);
                stbi_image_free(data);
    
                return pixmap(texture_pixels, texture_width, texture_height);
            }
    
            inline static auto from_vector(const ulx::file& image_file, ulx::u32 width, ulx::u32 height) -> pixmap {
                ulx::str file_path = image_file.get_file_path();
                NSVGimage* svg = nsvgParseFromFile(file_path.c_str(), "px", 96.0f);
                if (!svg) ulx::log::ulixerr("failed to parse svg (nanosvg): {}", file_path);
                NSVGrasterizer* rast = nsvgCreateRasterizer();
    
                width *= 2; height *= 2;
                ulx::f32 scale = std::min(
                    static_cast<ulx::f32>(width) / svg->width,
                    static_cast<ulx::f32>(height) / svg->height
                );
                ulx::u32 stride = ((width * 4 + 15) & ~15);
                ulx::vec<ulx::u8> texture_pixels(stride * height, 0);
    
                nsvgRasterize(rast, svg, 0, 0, scale, texture_pixels.data(), width, height, stride);
                nsvgDeleteRasterizer(rast);
                nsvgDelete(svg);
    
                return pixmap(texture_pixels, width, height);
            }
    
            inline static auto from_text(const ulx::str& text, const ulx::font font = ulx::font()) -> pixmap {
                const __uii::font::TextLayout& text_layout = __uii::font::layout_text(text, font);
    
                ulx::u32 width = static_cast<ulx::u32>(text_layout.total_width);
                ulx::u32 height = static_cast<ulx::u32>(text_layout.total_height);
                if (width == 0 || height == 0) ulx::log::ulixerr("failed to rasterize text (stb_truetype): empty text bounds");
    
                ulx::bytes pixels(static_cast<ulx::size>(width * height * 4));
                ulx::color color = font.get_color();
                ulx::u8 red = color.get_red(), green = color.get_green(), blue = color.get_blue();
                float alpha_scale = color.get_alpha() / 255.0f;
    
                // Stitch text bitmaps
                for (const __uii::font::TextLayout::GlyphPosition& glyph_position : text_layout.glyphs) {
                    const __uii::font::GlyphValue& glyph_value = __uii::font::glyph_cache.find(glyph_position.key)->second;
    
                    for (ulx::i32 glyph_y = 0; glyph_y < glyph_position.height; ++glyph_y) {
                        for (ulx::i32 glyph_x = 0; glyph_x < glyph_position.width; ++glyph_x) {
                            ulx::i32 position_x = glyph_position.x + glyph_x;
                            ulx::i32 position_y = glyph_position.y + glyph_y;
                            if (static_cast<ulx::u32>(position_x) >= width || static_cast<ulx::u32>(position_y) >= height)
                                continue;
    
                            ulx::u8 alpha = static_cast<ulx::u8>(glyph_value.bitmap[static_cast<ulx::size>(glyph_y * glyph_position.width + glyph_x)] * alpha_scale);
                            auto* dst = &pixels[static_cast<ulx::size>((position_y * width + position_x) * 4)];
                            dst[0] = red; dst[1] = green; dst[2] = blue; dst[3] = alpha;
                        }
                    }
                }
    
                return pixmap(std::move(pixels), width, height);
            }
    
        public:
            auto get_rect() const -> ulx::rect { return ulx::rect(0, 0, texture_width, texture_height); }
            auto get_pixel_size() const -> ulx::u64 { return texture_size; }
            auto get_pixels() const -> const ulx::vec<ulx::u8> { return texture_pixels; }
    };
}