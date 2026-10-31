#pragma once
#include "../log.hpp"
#include "../types.hpp"
#include "../font.hpp"
#include "../__required_libraries/freetype/freetype.hpp"
#include "ulix/pack.hpp"
#include "ulix/path.hpp"
#include "ulix/stablestr.hpp"
#include "ulix/winfunc.hpp"

namespace __uii::fgg {
    inline static UINT window_dpi;
    inline static auto is_fullwidth_punct(ulx::u32 cp) -> bool {
        return (cp >= 0x3001 && cp <= 0x3002) ||                                                  // 、。
               (cp >= 0x3008 && cp <= 0x3011 && cp % 2) ||                                        // 》」』】
               (cp == 0xFF01 || cp == 0xFF09 || cp == 0xFF09 || cp == 0xFF0C || cp == 0xFF0E) ||  // ！），．
               (cp == 0xFF1A || cp == 0xFF1B || cp == 0xFF0F) ||                                  // ：；？
               (cp == 0xFF3D) ||                                                                  // ］
               cp == 0x3000;                                                                      // fullwidth space
    }
    
    struct char_glyph {
        ulx::f32 width, height;
        ulx::f32 x_offset, y_offset;
        ulx::f32 advance;
        ulx::u32 baseline;
        ulx::bytes bitmap;
    };

    struct font_data {
        ulx::bytes bytes;
        FT_Face face;
    };

    inline static ulx::hashmap<ulx::font, ulx::f32> font_to_max_h;
    inline static ulx::hashmap<ulx::pathstr, font_data> font_cache;
    inline static FT_Library ft_library = nullptr;
    struct _init_struct {
        inline constexpr _init_struct() {
            ulx::log::expect(FT_Init_FreeType(&ft_library) == 0, "__inside_impl/font_glyph_getter.hpp: namespace __uii::fgg: struct _init_struct: _init_struct(): FT_Init_FreeType() != 0 (FreeType)");
        }
    } inline static _init;

    inline static auto load(const ulx::font& font) -> FT_Face {
        ulx::pathstr name = font.get_name();

        auto it = font_cache.find(name);
        if (it != font_cache.end())
            return it->second.face;

        ulx::pathstr font_path = name;
        if (!ulx::path(name).exists())
            font_path = "C:\\Windows\\Fonts\\" + name;

        ulx::path font_file(font_path);
        ulx::log::expect(font_file.exists(), "__inside_impl/font_glyph_getter.hpp: namespace __uii::fgg: load(const ulx::font&): font_file.exists() == false (FreeType)");

        ulx::bytes bytes = font_file.read_bytes();
        ulx::log::expect(!bytes.empty(), "__inside_impl/font_glyph_getter.hpp: namespace __uii::fgg: load(const ulx::font&): bytes.empty() == true (FreeType)");

        FT_Face face = nullptr;
        int face_index = 0;
        if (bytes.size() > 4 && bytes[0] == 't' && 
            bytes[1] == 't' && bytes[2] == 'c' && 
            bytes[3] == 'f') face_index = 0;

        ulx::log::expect(FT_New_Memory_Face(ft_library, bytes.data(), static_cast<FT_Long>(bytes.size()), face_index, &face) == 0, 
            "__inside_impl/font_glyph_getter.hpp: namespace __uii::fgg: load(const ulx::font&): FT_New_Memory_Face() != 0");

        // FT_Set_Pixel_Sizes(face, 0, font.get_size());
        FT_Set_Char_Size(face, 0, font.get_size() * 64, 0, 0);
        font_data data { std::move(bytes), face };

        return font_cache.emplace(name, std::move(data)).first->second.face;
    }

    inline static auto get_char_glyph(const ulx::font& font, ulx::u32 codepoint) -> char_glyph {
        FT_Face face = load(font);
    
        FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
        ulx::log::expect(glyph_index != 0, "__inside_impl/font_glyph_getter.hpp: namespace __uii::fgg: get_char_glyph(const ulx::font&, ulx::u32): glyph_index == 0");
    
        FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_LCD;
        ulx::log::expect(FT_Load_Glyph(face, glyph_index, load_flags) == 0, "__inside_impl/font_glyph_getter.hpp: namespace __uii::fgg: get_char_glyph(const ulx::font&, ulx::u32): FT_Load_Glyph() != 0");
    
        FT_GlyphSlot slot = face->glyph;
        FT_Bitmap& bitmap = slot->bitmap;
    
        ulx::f32 ascender_px = static_cast<ulx::f32>((face->size->metrics.ascender) / 64.0f);
        ulx::f32 descender_px = static_cast<ulx::f32>((-face->size->metrics.descender) / 64.0f);
        ulx::f32 box_height = ascender_px + descender_px;
    
        ulx::f32 pixel_width = bitmap.width / 3.0f;
        ulx::f32 pixel_height = bitmap.rows;
        ulx::f32 bitmap_top = slot->bitmap_top;
        ulx::f32 bitmap_left = slot->bitmap_left;
        ulx::f32 glyph_y_in_box = ascender_px - bitmap_top;

        bool is_fullwidth = is_fullwidth_punct(codepoint);
        ulx::f32 advance = slot->metrics.horiAdvance / 64.0f;
        if (is_fullwidth) advance = advance * 2.0f / 3.0f;
        
        ulx::f32 box_width = advance;
        ulx::bytes rgba(box_width * box_height * 4, 0);
        for (int y = 0; y < pixel_height; y++) {
            int box_y = glyph_y_in_box + y;
            if (box_y < 0 || box_y >= box_height) continue;
            for (int x = 0; x < pixel_width; x++) {
                int box_x = x;
                if (!is_fullwidth) box_x = bitmap_left + x;
                if (box_x < 0 || box_x >= box_width) continue;
    
                int r = bitmap.buffer[y * bitmap.pitch + x * 3 + 0];
                int g = bitmap.buffer[y * bitmap.pitch + x * 3 + 1];
                int b = bitmap.buffer[y * bitmap.pitch + x * 3 + 2];
                int alpha = std::max({r, g, b});
    
                int idx = (box_y * box_width + box_x) * 4;
                rgba[idx + 0] = static_cast<ulx::u8>(r);
                rgba[idx + 1] = static_cast<ulx::u8>(g);
                rgba[idx + 2] = static_cast<ulx::u8>(b);
                rgba[idx + 3] = static_cast<ulx::u8>(alpha);
            }
        }
    
        char_glyph glyph{};
        glyph.width = box_width;
        glyph.height = box_height;
        glyph.x_offset = 0;
        glyph.y_offset = 0;
        glyph.advance = advance;
        glyph.baseline = ascender_px;
        glyph.bitmap = std::move(rgba);
    
        return glyph;
    }

    inline auto decode_utf8(const ulx::str& text, ulx::u64& pos) -> ulx::u32 {
        ulx::u8 byte = static_cast<ulx::u8>(text[pos]);
        ulx::u32 codepoint;

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

    inline auto get_codepoint(const ulx::u8char& chr) -> ulx::u32 {
        ulx::u8 byte = static_cast<ulx::u8>(chr.array()[0]);

        ulx::u32 codepoint;
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

    inline static auto layout_char(const ulx::u8char& chr, const ulx::font& font) -> char_glyph {
        ulx::u32 codepoint = get_codepoint(chr);
        return get_char_glyph(font, codepoint);
    }

    inline static auto line_metrics(const ulx::font& font) -> ulx::pack<ulx::f32, ulx::f32, ulx::f32> {
        FT_Face face = load(font.make(ulx::wfn::physical(font.get_size(), window_dpi)));
        ulx::f32 ascent  = (face->size->metrics.ascender  + 63.0f) / 64.0f;
        ulx::f32 descent = (-face->size->metrics.descender + 63.0f) / 64.0f;
        ulx::f32 line_h  = ascent + descent;
        return {ascent, descent, line_h};
    }
}
