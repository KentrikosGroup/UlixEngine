#pragma once
#include "ulix/path.hpp"
#include "ulix/requires.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "font.hpp"
#include "__inside_impl/font_glyph_getter.hpp"
#include "log.hpp"
#include "size.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include "types.hpp"
#include "__required_libraries/stb_image/stb_image.hpp"
#include "__required_libraries/stb_image_write/stb_image_write.hpp"
#include "__required_libraries/resvg/resvg.hpp"

#define WUFFS_IMPLEMENTATION
#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__PNG
#define WUFFS_CONFIG__MODULE__JPEG
#define WUFFS_CONFIG__MODULE__BMP
#define WUFFS_CONFIG__MODULE__DEFLATE
#define WUFFS_CONFIG__MODULE__ZLIB
#define WUFFS_CONFIG__MODULE__CRC32
#define WUFFS_CONFIG__MODULE__ADLER32
#include <ulix/__required_libraries/wuffs/wuffs.hpp>


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

        public: inline constexpr static auto from_bitmap(const path& image) -> pixmap {
            pathstr file_path = image.str();
            auto raw = image.read_bytes();
            log::expect(!raw.empty(), "pixmap.hpp: namespace ulx: class pixmap: from_bitmap(const path&): raw.empty() == true");
        
            wuffs_base__slice_u8 raw_slice = wuffs_base__make_slice_u8(raw.data(), raw.size());
            int32_t fourcc = wuffs_base__magic_number_guess_fourcc(raw_slice, true);
        
            wuffs_base__image_decoder* dec = nullptr;
            wuffs_png__decoder png_dec;
            wuffs_jpeg__decoder jpeg_dec;
            wuffs_bmp__decoder bmp_dec;
            wuffs_base__status status;
        
            switch (fourcc) {
                case WUFFS_BASE__FOURCC__PNG:
                    status = wuffs_png__decoder__initialize(&png_dec, sizeof__wuffs_png__decoder(), WUFFS_VERSION, WUFFS_INITIALIZE__DEFAULT_OPTIONS);
                    dec = wuffs_png__decoder__upcast_as__wuffs_base__image_decoder(&png_dec);
                    break;
                case WUFFS_BASE__FOURCC__JPEG:
                    status = wuffs_jpeg__decoder__initialize(&jpeg_dec, sizeof__wuffs_jpeg__decoder(), WUFFS_VERSION, WUFFS_INITIALIZE__DEFAULT_OPTIONS);
                    dec = wuffs_jpeg__decoder__upcast_as__wuffs_base__image_decoder(&jpeg_dec);
                    break;
                case WUFFS_BASE__FOURCC__BMP:
                    status = wuffs_bmp__decoder__initialize(&bmp_dec, sizeof__wuffs_bmp__decoder(), WUFFS_VERSION, WUFFS_INITIALIZE__DEFAULT_OPTIONS);
                    dec = wuffs_bmp__decoder__upcast_as__wuffs_base__image_decoder(&bmp_dec);
                    break;

                // fallback to stb_image
                default:
                    i32 texture_width = 0, texture_height = 0, texture_channels = 0;
        
                    u8* data = stbi_load(file_path.cstr(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
                    log::expect(data, "pixmap.hpp: namespace ulx: class pixmap: from_bitmap(const path&): data == nullptr, stbi_failure_reason(): {}", stbi_failure_reason());
                    vec<u8> texture_pixels(data, data + texture_width * texture_height * 4);
                    stbi_image_free(data);
        
                    return pixmap(texture_pixels, texture_width, texture_height);
            }
            log::expect(!wuffs_base__status__is_error(&status), "pixmap.hpp: namespace ulx: class pixmap: from_bitmap(const path&): wuffs_base__status__is_error() == true, wuffs_base__status__message(): {}", wuffs_base__status__message(&status));
        
            wuffs_base__io_buffer src;
            src.data = wuffs_base__make_slice_u8(raw.data(), raw.size());
            src.meta.wi = raw.size();
            src.meta.ri = 0;
            src.meta.pos = 0;
            src.meta.closed = true;
        
            wuffs_base__image_config ic;
            status = dec->decode_image_config(&ic, &src);
            log::expect(!wuffs_base__status__is_error(&status), "pixmap.hpp: namespace ulx: class pixmap: from_bitmap(const path&): wuffs_base__status__is_error() == true, wuffs_base__status__message(): {}", wuffs_base__status__message(&status));
        
            uint32_t width = wuffs_base__pixel_config__width(&ic.pixcfg);
            uint32_t height = wuffs_base__pixel_config__height(&ic.pixcfg);
            wuffs_base__pixel_config pc;
            wuffs_base__pixel_config__set(&pc,
                WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL,
                WUFFS_BASE__PIXEL_SUBSAMPLING__NONE,
                width, height);
        
            size_t stride = 4 * width;
            size_t buf_size = stride * height;
        
            vec<u8> texture_pixels(buf_size);
            wuffs_base__pixel_buffer pb;
            wuffs_base__slice_u8 slice = wuffs_base__make_slice_u8(
                texture_pixels.data(), buf_size);
            wuffs_base__status status2 = wuffs_base__pixel_buffer__set_from_slice(&pb, &pc, slice);
            log::expect(!wuffs_base__status__is_error(&status2), "pixmap.hpp: namespace ulx: class pixmap: from_bitmap(const path&): wuffs_base__status__is_error() == true, wuffs_base__status__message(): {}", wuffs_base__status__message(&status));
        
            wuffs_base__range_ii_u64 wb = dec->workbuf_len();
            vec<u8> work_mem(wb.max_incl, 0);
            wuffs_base__slice_u8 workbuf = wuffs_base__make_slice_u8(work_mem.data(), work_mem.size());
        
            status = dec->decode_frame(&pb, &src, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf, nullptr);
            log::expect(!wuffs_base__status__is_error(&status), "pixmap.hpp: namespace ulx: class pixmap: from_bitmap(const path&): wuffs_base__status__is_error() == true, wuffs_base__status__message(): {}", wuffs_base__status__message(&status));
        
            return pixmap(std::move(texture_pixels), width, height);
        }

        public: inline constexpr static auto from_vector(const path& image, u32 width, u32 height) -> pixmap {
            pathstr file_path = image.str();

            resvg_options* options = resvg_options_create();
            log::expect(options, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const path&, u32, u32): resvg_options_create() == nullptr");

            resvg_options_load_system_fonts(options);
            resvg_options_set_dpi(options, 96.0f);

            resvg_render_tree* tree = nullptr;
            int result = resvg_parse_tree_from_file(file_path.cstr(), options, &tree);
            log::expect(resvg_parse_tree_from_file(file_path.cstr(), options, &tree) == RESVG_OK, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const path&, u32, u32): resvg_parse_tree_from_file() != RESVG_OK");

            resvg_size size = resvg_get_image_size(tree);
            f32 svg_width  = static_cast<f32>(size.width);
            f32 svg_height = static_cast<f32>(size.height);
            log::expect(svg_width > 0.0f, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const path&, u32, u32): svg_width <= 0.0f");
            log::expect(svg_height > 0.0f, "pixmap.hpp: namespace ulx: class pixmap: from_vector(const path&, u32, u32): svg_height <= 0.0f");

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

        public: inline constexpr auto save(path&& f) const -> void { 
            stbi_write_png(f.str().cstr(), texture_width, texture_height, 4, reinterpret_cast<const void*>(texture_pixels.data()), texture_width * 4);
        }
    };
}
