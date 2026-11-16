#pragma once
#include <cstddef>
#include <mutex>
#include <ulix/log.hpp>
#include <ulix/types.hpp>
#include <ulix/font.hpp>
#include <ulix/pack.hpp>
#include <ulix/path.hpp>
#include <ulix/stablestr.hpp>
#include <ulix/winfunc.hpp>
#include <ulix/guard.hpp>
#include <ulix/__inside_impl/log_utils.hpp>
#include <ulix/__required_libraries/freetype/freetype.hpp>

namespace ulx::fgg {
    inline static auto is_fullwidth_punct(u32 cp) -> bool {
        return (cp >= 0x3001 && cp <= 0x3002) ||                                                  // 、。
               (cp >= 0x3008 && cp <= 0x3011 && cp % 2) ||                                        // 》」』】
               (cp == 0xFF01 || cp == 0xFF09 || cp == 0xFF0C || cp == 0xFF0E) ||                  // ！），．
               (cp == 0xFF1A || cp == 0xFF1B || cp == 0xFF0F) ||                                  // ：；？
               (cp == 0xFF3D) ||                                                                  // ］
               cp == 0x3000;                                                                      // fullwidth space
    }

    struct char_glyph {
        f32   width,
              height;
        f32   x_offset,
              y_offset;
        f32   advance;
        bytes bitmap;
    };

    struct font_data {
        bytes bytes;
        FT_Face face;
    };

    inline static std::mutex cache_mutex;
    inline static hashmap<pathstr, font_data> font_cache;
    inline static FT_Library ft_library = nullptr;
    inline static auto init_freetype = []() -> std::nullptr_t {
        FT_Error error_code = FT_Init_FreeType(&ft_library);
        log::uassert(error_code == 0, "initialize freetype library failed, error code: {}", error_code);
        return nullptr;
    }();

    inline static auto get_path(const font& font) -> pathstr {
        pathstr font_path = font.get_path();
        if (!path(font_path).exists())
            font_path = "C:\\Windows\\Fonts\\" + font_path;

        path font_file(font_path);
        if (!font_file.exists()) font_file = "C:\\Windows\\Fonts\\arial.ttf";

        return font_path;
    }

    inline static auto load(const font& font) -> FT_Face {
        pathstr fontpath = font.get_path();

        FT_Face cached_face = nullptr;
        bool found = false;
        ulx::guard(cache_mutex, [&]() {
            auto it = font_cache.find(fontpath);
            if (it != font_cache.end()) {
                cached_face = it->second.face;
                found = true;
            }
        });
        if (found) return cached_face;

        pathstr new_path = fontpath;
        if (!path(fontpath).exists())
            new_path = "C:\\Windows\\Fonts\\" + fontpath;

        path font_file(new_path);
        if (!font_file.exists()) {
            log::uwarn("font file {} doesn't exists, fall back to C:\\Windows\\Fonts\\arial.ttf", fontpath.cstr());
            font_file = "C:\\Windows\\Fonts\\arial.ttf";
        }

        bytes bytes = font_file.read_bytes();
        log::uassert(!bytes.empty(), "the data of the font file {} is empty", font_file.str().cstr());

        FT_Face face = nullptr;
        int face_index = 0;
        if (bytes.size() > 4 && bytes[0] == 't' &&
            bytes[1] == 't' && bytes[2] == 'c' &&
            bytes[3] == 'f') face_index = 0;

        FT_Error error_code = FT_New_Memory_Face(ft_library, bytes.data(), static_cast<FT_Long>(bytes.size()), face_index, &face);
        log::uassert(error_code == 0,
            "create new font memory face from the font file {} failed, error code: {}",
            font_file.str().cstr(), error_code);

        error_code = FT_Set_Char_Size(face, 0, font.get_size() * 128.0f, 0, 0);
        log::uassert(error_code == 0,
            "set font char size failed, the font size {} may be incorrect, font file: {}, error code: {}",
            font.get_size(), font_file.str().cstr(), error_code);

        font_data data { std::move(bytes), face };
        return ulx::guard(cache_mutex, [&]() {
            return font_cache.emplace(fontpath, std::move(data)).first->second.face;
        });
    }

    inline static auto get_char_glyph(const font& font, u32 codepoint) -> char_glyph {
        FT_Face face = load(font);

        FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
        if (glyph_index == 0) {
            log::uwarn("get char index from codepoint failed, the codepoint {} may not exist in the font file {}, the char will continue to be generated as ▯",
                codepoint, font.get_path().cstr());
            glyph_index = FT_Get_Char_Index(face, 0x25AF);
        }

        FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL;

        FT_Error error_code = FT_Load_Glyph(face, glyph_index, load_flags);
        log::uassert(error_code == 0,
            "load char glpyh failed, the font file {} may have been maliciously damaged, glpyh index: {}",
            get_path(font).cstr(), glyph_index);

        FT_GlyphSlot slot = face->glyph;
        FT_Bitmap& bitmap = slot->bitmap;

        f32 ascender_px = static_cast<f32>((face->size->metrics.ascender) / 64.0f);
        f32 descender_px = static_cast<f32>((-face->size->metrics.descender) / 64.0f);
        f32 box_height = ascender_px + descender_px;

        f32 bitmap_top = slot->bitmap_top;
        f32 bitmap_left = slot->bitmap_left;
        f32 glyph_y_in_box = ascender_px - bitmap_top;

        bool is_fullwidth = is_fullwidth_punct(codepoint);
        f32 advance = slot->metrics.horiAdvance / 64.0f;
        if (is_fullwidth) advance = advance * 2.0f / 3.0f;

        f32 box_width = advance;
        bytes rgba(box_width * box_height * 4, 0);
        for (int y = 0; y < bitmap.rows; y++) {
            f32 box_y = glyph_y_in_box + y;
            if (box_y < 0 || box_y >= box_height) continue;

            for (int x = 0; x < bitmap.width; x++) {
                f32 box_x = bitmap_left + x;
                if (box_x < 0 || box_x >= box_width) continue;

                u8 coverage = bitmap.buffer[y * bitmap.pitch + x];
                u64 idx = (u64(box_y) * u64(box_width) + u64(box_x)) * 4;

                rgba[idx + 0] = coverage;
                rgba[idx + 1] = coverage;
                rgba[idx + 2] = coverage;
                rgba[idx + 3] = coverage;
            }
        }

        char_glyph glyph{};
        glyph.width = box_width;
        glyph.height = box_height;
        glyph.x_offset = 0;
        glyph.y_offset = 0;
        glyph.advance = advance;
        glyph.bitmap = std::move(rgba);
        return glyph;
    }

    inline auto decode_utf8(const ulx::str& text, ulx::u64& pos) -> ulx::u32 {
        u8 byte = u8(text[pos]);
        u32 codepoint;

        if (byte < 0x80) {
            codepoint = byte;
            pos += 1;
        } else if (byte < 0xE0) {
            codepoint = byte & 0x1F;
            codepoint = (codepoint << 6) | (text[pos + 1] & 0x3F);
            pos += 2;
        } else if (byte < 0xF0) {
            codepoint = byte & 0x0F;
            codepoint = (codepoint << 6) | (text[pos + 1] & 0x3F);
            codepoint = (codepoint << 6) | (text[pos + 2] & 0x3F);
            pos += 3;
        } else {
            codepoint = byte & 0x07;
            codepoint = (codepoint << 6) | (text[pos + 1] & 0x3F);
            codepoint = (codepoint << 6) | (text[pos + 2] & 0x3F);
            codepoint = (codepoint << 6) | (text[pos + 3] & 0x3F);
            pos += 4;
        }

        return codepoint;
    }

    inline auto get_codepoint(const u8char& chr) -> u32 {
        u8 byte = static_cast<u8>(chr.array()[0]);

        u32 codepoint;
        if (byte < 0x80) {
            codepoint = byte;
        } else if (byte < 0xE0) {
            codepoint = byte & 0x1F;
            codepoint = (codepoint << 6) | (chr.array()[1] & 0x3F);
        } else if (byte < 0xF0) {
            codepoint = byte & 0x0F;
            codepoint = (codepoint << 6) | (chr.array()[1] & 0x3F);
            codepoint = (codepoint << 6) | (chr.array()[2] & 0x3F);
        } else {
            codepoint = byte & 0x07;
            codepoint = (codepoint << 6) | (chr.array()[1] & 0x3F);
            codepoint = (codepoint << 6) | (chr.array()[2] & 0x3F);
            codepoint = (codepoint << 6) | (chr.array()[3] & 0x3F);
        }

        return codepoint;
    }

    inline static auto layout_char(const u8char& chr, const font& font) -> char_glyph {
        u32 codepoint = get_codepoint(chr);
        return get_char_glyph(font, codepoint);
    }

    inline static auto line_metrics(const font& font) -> pack<f32, f32, f32> {
        FT_Face face = load(font.make(wfn::physical(font.get_size())));
        f32 ascent  = (face->size->metrics.ascender) / 128.0f;
        f32 descent = (-face->size->metrics.descender) / 128.0f;
        f32 line_h  = ascent + descent;
        return {ascent, descent, line_h};
    }
}
