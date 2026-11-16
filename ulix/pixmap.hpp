#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <ulix/path.hpp>
#include <ulix/requires.hpp>
#include <ulix/font.hpp>
#include <ulix/size.hpp>
#include <ulix/stablestr.hpp>
#include <ulix/types.hpp>
#include <ulix/__inside_impl/font_glyph_getter.hpp>
#include <ulix/__inside_impl/pixmap_decodes.hpp>
#include <ulix/__inside_impl/svg_decodes.hpp>
#include <ulix/__required_libraries/fpng/fpng.hpp>


namespace ulx {
    class pixmap {
        u32 m_width, m_height;
        u64 m_size;
        vec<u8> m_pixels;

        public: inline constexpr pixmap() = default;
        public: inline constexpr pixmap(vec<u8> pixels, u32 width, u32 height) : m_pixels(pixels), m_width(width), m_height(height), m_size(pixels.size()) {}
        public: inline constexpr pixmap(u32 width, u32 height) : m_pixels(width * height * 4), m_width(width), m_height(height), m_size(width * height * 4) {}

        public: inline constexpr static auto from_bitmap(const path& image) -> pixmap {
            auto decoded = pds::decode(image);
            return pixmap(std::move(decoded.get<0>()), decoded.get<1>(), decoded.get<2>());
        }

        public: inline constexpr static auto from_vector(const path& image, u32 width, u32 height) -> pixmap {
            auto generated = sds::generate(image, width, height);
            return pixmap(std::move(generated.get<0>()), generated.get<1>(), generated.get<2>());
        }

        public: inline constexpr static auto from_char(const u8char& chr, const font& f = font()) -> pixmap {
            fgg::char_glyph glyph = fgg::layout_char(chr, f);

            u32 width = static_cast<u32>(glyph.width);
            u32 height = static_cast<u32>(glyph.height);
            log::expect(width > 0, "pixmap.hpp: namespace ulx: class pixmap: from_char(const u8char&, const font&): width == 0");
            log::expect(height > 0, "pixmap.hpp: namespace ulx: class pixmap: from_char(const u8char&, const font&): height == 0");

            auto pm = pixmap(std::move(glyph.bitmap), width, height);
            return pm;
        }

        public: inline constexpr auto size() const -> size { return {static_cast<f32>(m_width), static_cast<f32>(m_height)}; }
        public: inline constexpr auto w() const -> u32 { return m_width; }
        public: inline constexpr auto h() const -> u32 { return m_height; }
        public: inline constexpr auto pixel_size() const -> u64 { return m_size; }
        public: inline constexpr auto pixels() -> vec<ulx::u8>& { return m_pixels; }

        public: inline constexpr auto savepng(const path& p) const -> void {
            bytes out;
            fpng::fpng_encode_image_to_memory(m_pixels.data(), m_width, m_height, 4, out);
            p.write(out);
        }
    };
}
