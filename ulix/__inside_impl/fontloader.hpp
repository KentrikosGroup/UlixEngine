#pragma once

#include "../log.hpp"
#include "../file.hpp"
#include "../types.hpp"
#include "../font.hpp"
#include "../__require_libraries/skarupke/flat_hash_map.hpp"
#include "../__require_libraries/stb_truetype/stb_truetype.hpp"
#include <functional>

namespace __uii::font {
    struct glyph_key {
        ulx::u32 font_id;
        ulx::u32 codepoint;
        ulx::u32 size;
    };

    struct glyph_value {
        ulx::i32 width, height;
        ulx::i32 x_offset, y_offset;
        ulx::i32 advance;
        ulx::bytes bitmap;
    };

    struct text_layout_key {
        ulx::str text;
        ulx::u32 font_id;
        ulx::u32 size;
    };

    struct text_layout {
        struct glyph_position {
            ulx::u32 codepoint;
            ulx::i32 x, y;
            ulx::i32 width, height;
            glyph_key key;
        };

        ulx::i32 total_width, total_height;
        ulx::i32 baseline_y;
        ulx::vec<glyph_position> glyphs;
    };
}

template<> struct std::equal_to<__uii::font::glyph_key> {
    auto operator()(const __uii::font::glyph_key& a, const __uii::font::glyph_key& b) const -> bool {
        return a.font_id == b.font_id &&
                a.codepoint == b.codepoint &&
                a.size == b.size;
    }
};

template<> struct std::equal_to<__uii::font::text_layout_key> {
    auto operator()(const __uii::font::text_layout_key& a, const __uii::font::text_layout_key& b) const -> bool {
        return a.text == b.text &&
                a.font_id == b.font_id &&
                a.size == b.size;
    }
};

template<> struct std::hash<__uii::font::glyph_key> {
    ulx::u64 operator()(const __uii::font::glyph_key& k) const noexcept {
        ulx::u64 h = k.font_id;
        h ^= k.codepoint << 1;
        h ^= k.size << 2;
        return h;
    }
};

template<> struct std::hash<__uii::font::text_layout_key> {
    ulx::u64 operator()(const __uii::font::text_layout_key& k) const noexcept {
        ulx::u64 h = std::hash<ulx::str>{}(k.text);
        h ^= k.font_id << 1;
        h ^= k.size << 2;
        return h;
    }
};


namespace __uii::font {
    inline static ska::flat_hash_map<ulx::str, ulx::u32> font_name_to_id;
    inline static std::vector<ulx::str> font_id_to_name;
    inline static ulx::u32 next_font_id = 0;
    inline static ska::flat_hash_map<ulx::font, ulx::i32> font_to_max_h;

    struct font_data {
        ulx::bytes bytes;
        stbtt_fontinfo info;
    };

    inline auto get_font_id(const ulx::str& font_name) -> ulx::u32 {
        auto it = font_name_to_id.find(font_name);
        if (it != font_name_to_id.end())
            return it->second;

        ulx::u32 id = next_font_id++;
        font_name_to_id[font_name] = id;
        font_id_to_name.push_back(font_name);
        return id;
    }

    inline static ska::flat_hash_map<ulx::str, font_data> font_cache;
    inline static auto load(const ulx::font& font) -> stbtt_fontinfo* {
        ulx::str key = font.get_name();

        auto it = font_cache.find(key);
        if (it != font_cache.end())
            return &it->second.info;

        ulx::str font_path = key; if (!ulx::file(key).exists()) font_path = "C:\\Windows\\Fonts\\" + key;
        ulx::file font_file(font_path); if (!font_file.exists()) ulx::log::err("failed to find font file: {}", font_path);
        ulx::bytes bytes = font_file.read_bytes(); if (bytes.empty()) ulx::log::err("failed to read font file: {}", font_path);

        font_data data { .bytes = std::move(bytes) };
        int font_offset = 0;
        if (data.bytes.size() > 4 && 
            data.bytes[0] == 't' && data.bytes[1] == 't' && 
            data.bytes[2] == 'c' && data.bytes[3] == 'f') {
            font_offset = stbtt_GetFontOffsetForIndex(data.bytes.data(), 0);
        }
        if (!stbtt_InitFont(&data.info, data.bytes.data(), font_offset))
            ulx::log::err("failed to initialize font (stb_truetype): {}", font_path);

        return &font_cache.emplace(key, std::move(data)).first->second.info;
    }

    inline static auto get_scale(const ulx::font& font) -> ulx::f32 {
        stbtt_fontinfo* info = load(font);
        return stbtt_ScaleForPixelHeight(info, static_cast<ulx::f32>(font.get_size()));
    }

    inline static ulx::hashmap<glyph_key, glyph_value> glyph_cache;
    inline static ulx::hashmap<text_layout_key, text_layout> line_layout_cache;

    inline static auto get_glyph(const ulx::font& font, ulx::u32 codepoint, ulx::f32 scale) -> const glyph_value& {
        ulx::u32 font_id = get_font_id(font.get_name());
        glyph_key glyph_key { font_id, codepoint, font.get_size() };

        auto it = glyph_cache.find(glyph_key);
        if (it != glyph_cache.end())
            return it->second;

        stbtt_fontinfo* fontinfo = __uii::font::load(font);
        int width, height, x_offset, y_offset;

        ulx::u8* bitmap = stbtt_GetCodepointBitmap(fontinfo, scale, scale, codepoint, &width, &height, &x_offset, &y_offset);
        if (!bitmap) ulx::log::err("failed to get glyph bitmap (stb_truetype)");

        int advance; stbtt_GetCodepointHMetrics(fontinfo, codepoint, &advance, nullptr);
        glyph_value glyph_value{};
        glyph_value.width = width;
        glyph_value.height = height;
        glyph_value.x_offset = x_offset;
        glyph_value.y_offset = y_offset;
        glyph_value.advance = static_cast<int>(advance * scale);
        glyph_value.bitmap.assign(bitmap, bitmap + width * height);
        stbtt_FreeBitmap(bitmap, nullptr);

        return glyph_cache.emplace(glyph_key, std::move(glyph_value)).first->second;
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
    
    inline static auto layout_text(const ulx::str& text, const ulx::font& font) -> const text_layout& {
        ulx::f32 scale = __uii::font::get_scale(font);
        ulx::u32 font_id = get_font_id(font.get_name());
        text_layout_key text_layout_key { text, font_id, font.get_size() };
    
        auto it = line_layout_cache.find(text_layout_key);
        if (it != line_layout_cache.end())
            return it->second;
    
        text_layout layout;
        stbtt_fontinfo* fontinfo = __uii::font::load(font);
    
        std::vector<ulx::u32> codepoints;
        for (ulx::u64 i = 0; i < text.size(); ) {
            codepoints.push_back(decode_utf8(text, i));
        }
        
        layout.glyphs.reserve(codepoints.size());
        ulx::i32 current_x = 0;
        ulx::i32 min_x = INT_MAX, max_x = INT_MIN;
        ulx::i32 min_y = INT_MAX, max_y = INT_MIN;
        
        for (ulx::u64 i = 0; i < codepoints.size(); ++i) {
            ulx::u32 codepoint = codepoints[i];
            const glyph_value& glyph = get_glyph(font, codepoint, scale);
    
            text_layout::glyph_position glyph_position;
            glyph_position.codepoint = codepoint;
            glyph_position.x = current_x + glyph.x_offset;
            glyph_position.y = glyph.y_offset;
            glyph_position.width = glyph.width;
            glyph_position.height = glyph.height;
            glyph_position.key = glyph_key{ font_id, codepoint, font.get_size() };
    
            min_x = std::min(min_x, glyph_position.x);
            max_x = std::max(max_x, glyph_position.x + glyph_position.width);
            min_y = std::min(min_y, glyph_position.y);
            max_y = std::max(max_y, glyph_position.y + glyph_position.height);
    
            current_x += glyph.advance;
            if (i + 1 < codepoints.size()) {
                current_x += static_cast<ulx::i32>(
                    stbtt_GetCodepointKernAdvance(fontinfo, codepoint, codepoints[i + 1]) * scale
                );
            }
    
            layout.glyphs.push_back(std::move(glyph_position));
        }
    
        layout.total_width = max_x - min_x;
        layout.total_height = max_y - min_y;
        layout.baseline_y = -min_y;
    
        for (auto& glyph_position : layout.glyphs) {
            glyph_position.x -= min_x;
            glyph_position.y -= min_y;
        }
    
        return line_layout_cache.emplace(text_layout_key, std::move(layout)).first->second;
    }

    inline static auto max_height_of(const ulx::font& font) -> ulx::i32 {
        stbtt_fontinfo* info = load(font);
        if (font_to_max_h.find(font) != font_to_max_h.end())
            return font_to_max_h[font];
        
        ulx::f32 scale = get_scale(font);
        const unsigned char* data = info->data;
        
        int font_offset = 0;
        if (data[0] == 't' && data[1] == 't' && data[2] == 'c' && data[3] == 'f') {
            font_offset = stbtt_GetFontOffsetForIndex(data, 0);
            if (font_offset < 0) {
                int ascent, descent, line_gap;
                stbtt_GetFontVMetrics(info, &ascent, &descent, &line_gap);
                return static_cast<ulx::i32>(ascent * scale * 0.82f);
            }
        }
        
        ulx::i32 max_h = 0;
        const unsigned char* tables = data + font_offset + 12;
        int num_tables = *(unsigned short*)(data + font_offset + 4);
        num_tables = ((num_tables & 0xFF) << 8) | ((num_tables >> 8) & 0xFF);
        
        int os2_offset = -1;
        for (int i = 0; i < num_tables; i++) {
            const unsigned char* entry = tables + i * 16;
            char tag[5] = {0};
            tag[0] = entry[0]; tag[1] = entry[1]; tag[2] = entry[2]; tag[3] = entry[3];
            
            if (strcmp(tag, "OS/2") == 0) {
                os2_offset = (entry[8] << 24) | (entry[9] << 16) | (entry[10] << 8) | entry[11];
                break;
            }
        }
        
        if (os2_offset < 0) {
            int ascent, descent, line_gap;
            stbtt_GetFontVMetrics(info, &ascent, &descent, &line_gap);
            max_h = static_cast<ulx::i32>(ascent * scale * 0.78f);
        }
        
        const unsigned char* os2_data = data + font_offset + os2_offset;
        ulx::u16 cap_height = *(ulx::u16*)(os2_data + 86);
        
        ulx::u16 cap = *(ulx::u16*)(os2_data + 88);
        cap = (cap >> 8) | (cap << 8);
        
        max_h = static_cast<ulx::i32>(cap * scale);
        font_to_max_h[font] = max_h;
        return max_h;
    }
}
