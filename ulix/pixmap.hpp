#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "font.hpp"
#include "__inside_impl/font_glyph_getter.hpp"
#include "file.hpp"
#include "log.hpp"
#include "size.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include "types.hpp"
#include "__required_libraries/stb_image/stb_image.hpp"
#include "__required_libraries/resvg/resvg.hpp"
#include "__required_libraries/wuffs/wuffs.hpp"


namespace ulx {
    class pixmap {
        u32 texture_width;
        u32 texture_height;
        u64 texture_size;
        vec<u8> texture_pixels;
        u32 baseln = 0;
    
        public: inline constexpr pixmap() = default;
        public: inline constexpr pixmap(vec<u8> pixels, u32 width, u32 height) : texture_pixels(pixels), texture_width(width), texture_height(height), texture_size(pixels.size()) {}
        public: inline constexpr pixmap(u32 width, u32 height) : texture_pixels(width * height * 4), texture_width(width), texture_height(height), texture_size(width * height * 4) {}
            
        public: inline constexpr static auto from_bitmap(const file& image_file) -> pixmap {
            pathstr file_path = image_file.path();
            i32 texture_width = 0, texture_height = 0, texture_channels = 0;

            u8* data = stbi_load(file_path.cstr(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
            log::expect(data, "pixmap.hpp: namespace ulx: class pixmap: from_bitmap(const file&): data == nullptr, stb failure reason: {}", stbi_failure_reason());
            vec<u8> texture_pixels(data, data + texture_width * texture_height * 4);
            stbi_image_free(data);

            return pixmap(texture_pixels, texture_width, texture_height);
        }

        public: inline constexpr static auto from_vector(const file& image_file, u32 width, u32 height) -> pixmap {
            pathstr file_path = image_file.path();
            
            resvg_options* options = resvg_options_create();
            log::expect(options, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const file&, u32, u32): resvg_options_create() == nullptr");
        
            resvg_options_load_system_fonts(options);
            resvg_options_set_dpi(options, 96.0f);
        
            resvg_render_tree* tree = nullptr;
            int result = resvg_parse_tree_from_file(file_path.cstr(), options, &tree);
            log::expect(resvg_parse_tree_from_file(file_path.cstr(), options, &tree) == RESVG_OK, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const file&, u32, u32): resvg_parse_tree_from_file() != RESVG_OK");
        
            resvg_size size = resvg_get_image_size(tree);
            f32 svg_width  = static_cast<f32>(size.width);
            f32 svg_height = static_cast<f32>(size.height);
            log::expect(svg_width > 0.0f, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const file&, u32, u32): svg_width <= 0.0f");
            log::expect(svg_height > 0.0f, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const file&, u32, u32): svg_height <= 0.0f");

            u32 canvas_w = width * 2;
            u32 canvas_h = height * 2;            
            f32 scale = std::min(
                static_cast<f32>(canvas_w) / svg_width,
                static_cast<f32>(canvas_h) / svg_height
            );
        
            f32 offset_x = (static_cast<f32>(canvas_w) - svg_width * scale) * 0.5f;
            f32 offset_y = (static_cast<f32>(canvas_h) - svg_height * scale) * 0.5f;
        
            resvg_transform transform = {};
            transform.a = scale;
            transform.b = 0.0f;
            transform.c = 0.0f;
            transform.d = scale;
            transform.e = offset_x;
            transform.f = offset_y;
        
            u32 stride = canvas_w * 4;
            vec<u8> pixels(stride * canvas_h, 0);
        
            resvg_render(tree, transform, canvas_w, canvas_h, reinterpret_cast<char*>(pixels.data()));
            for (u64 i = 0; i < pixels.size(); i += 4) {
                u8 alpha = pixels[i + 3];
                if (alpha != 0 && alpha != 255) {
                    pixels[i + 0] = static_cast<u8>(
                        std::min(255, (pixels[i + 0] * 255) / alpha));
                    pixels[i + 1] = static_cast<u8>(
                        std::min(255, (pixels[i + 1] * 255) / alpha));
                    pixels[i + 2] = static_cast<u8>(
                        std::min(255, (pixels[i + 2] * 255) / alpha));
                }
            }
        
            resvg_tree_destroy(tree);
            resvg_options_destroy(options);
            return pixmap(std::move(pixels), canvas_w, canvas_h);
        }
        
        public: inline constexpr static auto from_char(const u8char& chr, const font& f = font()) -> pixmap {
            __uii::fgg::char_glyph glyph = __uii::fgg::layout_char(chr, f);
        
            u32 width = static_cast<u32>(glyph.width);
            u32 height = static_cast<u32>(glyph.height);
            log::expect(width > 0, "pixmap.hpp: namespace ulx: class pixmap: from_char(const u8char&, const font&): width == 0");
            log::expect(height > 0, "pixmap.hpp: namespace ulx: class pixmap: from_char(const u8char&, const font&): height == 0");    
        
            bytes pixels(static_cast<u64>(width * height * 4));
            color color = f.get_color();
            u8 red = color.red(), green = color.green(), blue = color.blue();
            float alpha_scale = color.alpha() / 255.0f;
            
            for (i32 glyph_y = 0; glyph_y < glyph.height; glyph_y++) {
                for (i32 glyph_x = 0; glyph_x < glyph.width; glyph_x++) {
                    auto* src = &glyph.bitmap[static_cast<u64>((glyph_y * glyph.width + glyph_x) * 4)];
                    u8 r = src[0], g = src[1], b = src[2], a = src[3];

                    u8 final_alpha = static_cast<u8>(a * alpha_scale);
                    
                    auto* dst = &pixels[static_cast<u64>((glyph_y * width + glyph_x) * 4)];
                    dst[0] = red;
                    dst[1] = green;
                    dst[2] = blue;
                    dst[3] = final_alpha;
                }
            }
        
            auto pm = pixmap(std::move(pixels), width, height);
            pm.baseln = glyph.baseline;
            return pm;
        }

        public: inline constexpr auto size() const -> size { return {static_cast<f32>(texture_width), static_cast<f32>(texture_height)}; }
        public: inline constexpr auto width() const -> u32 { return texture_width; }
        public: inline constexpr auto height() const -> u32 { return texture_height; }
        public: inline constexpr auto pixel_size() const -> u64 { return texture_size; }
        public: inline constexpr auto pixels() -> vec<ulx::u8>& { return texture_pixels; }
        public: inline constexpr auto baseline() const -> u32 { return baseln; }
    };
}