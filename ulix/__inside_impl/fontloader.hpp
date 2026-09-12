#pragma once

#include <file.hpp>
#include <types.hpp>
#include <font.hpp>
#include <algorithm>
#include <__require_libraries/skarupke/flat_hash_map.hpp>
#include <__require_libraries/stb_truetype/stb_truetype.hpp>

namespace __uii::font {
    struct GlyphKey {
        ulx::u32 font_id;
        ulx::u32 codepoint;
        ulx::u32 size;
    };

    struct GlyphValue {
        ulx::i32 width, height;
        ulx::i32 xoff, yoff;
        ulx::i32 advance;
        ulx::bytes bitmap;
    };

    struct GlyphKeyHash {
        std::size_t operator()(const GlyphKey& k) const noexcept {
            std::size_t h = k.font_id;
            h ^= k.codepoint << 1;
            h ^= k.size << 2;
            return h;
        }
    };

    struct TextLayoutKey {
        ulx::str text;
        ulx::u32 font_id;
        ulx::u32 size;
    };

    struct TextLayout {
        struct GlyphPosition {
            ulx::u32 codepoint;
            ulx::i32 x, y;
            ulx::i32 width, height;
            GlyphKey key;
        };

        ulx::i32 total_width, total_height;
        ulx::i32 baseline_y;
        std::vector<GlyphPosition> glyphs;
    };

    struct TextLayoutKeyHash {
        std::size_t operator()(const TextLayoutKey& k) const noexcept {
            std::size_t h = std::hash<ulx::str>{}(k.text);
            h ^= k.font_id << 1;
            h ^= k.size << 2;
            return h;
        }
    };
}

namespace std {
    template<> struct equal_to<__uii::font::GlyphKey> {
        auto operator()(const __uii::font::GlyphKey& a, const __uii::font::GlyphKey& b) const -> bool {
            return a.font_id == b.font_id &&
                   a.codepoint == b.codepoint &&
                   a.size == b.size;
        }
    };

    template<> struct equal_to<__uii::font::TextLayoutKey> {
        auto operator()(const __uii::font::TextLayoutKey& a, const __uii::font::TextLayoutKey& b) const -> bool {
            return a.text == b.text &&
                   a.font_id == b.font_id &&
                   a.size == b.size;
        }
    };
}


namespace __uii::font {
    inline static ska::flat_hash_map<ulx::str, ulx::u32> font_name_to_id;
    inline static std::vector<ulx::str> font_id_to_name;
    inline static ulx::u32 next_font_id = 0;

    struct FontData {
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

    inline static ska::flat_hash_map<ulx::str, FontData> font_cache;
    inline static auto load(const ulx::font& font) -> stbtt_fontinfo* {
        ulx::str key = font.get_name();

        auto it = font_cache.find(key);
        if (it != font_cache.end())
            return &it->second.info;

        ulx::str font_path = key; if (!ulx::file(key).exists()) font_path = "C:\\Windows\\Fonts\\" + key;
        ulx::file font_file(font_path); if (!font_file.exists()) ulx::log::ulixerr("failed to find font file: {}", font_path);
        ulx::bytes bytes = font_file.read_bytes(); if (bytes.empty()) ulx::log::ulixerr("failed to read font file: {}", font_path);

        FontData data { .bytes = std::move(bytes) };
        if (!stbtt_InitFont(&data.info, data.bytes.data(), 0))
            ulx::log::ulixerr("failed to initialize font (stb_truetype): {}", font_path);

        return &font_cache.emplace(key, std::move(data)).first->second.info;
    }

    inline static auto get_scale(const ulx::font& font) -> float {
        stbtt_fontinfo* info = load(font);
        return stbtt_ScaleForPixelHeight(info, static_cast<ulx::f32>(font.get_size()));
    }

    inline static ska::flat_hash_map<GlyphKey, GlyphValue, GlyphKeyHash> glyph_cache;
    inline static ska::flat_hash_map<TextLayoutKey, TextLayout, TextLayoutKeyHash> line_layout_cache;

    inline static auto get_glyph(const ulx::font& font, ulx::u32 codepoint, float scale) -> const GlyphValue& {
        ulx::u32 font_id = get_font_id(font.get_name());
        GlyphKey glyph_key { font_id, codepoint, font.get_size() };

        auto it = glyph_cache.find(glyph_key);
        if (it != glyph_cache.end())
            return it->second;

        stbtt_fontinfo* fontinfo = __uii::font::load(font);
        int width, height, x_offset, y_offset;

        ulx::u8* bitmap = stbtt_GetCodepointBitmap(fontinfo, scale, scale, codepoint, &width, &height, &x_offset, &y_offset);
        if (!bitmap) ulx::log::ulixerr("failed to get glyph bitmap (stb_truetype)");

        int advance; stbtt_GetCodepointHMetrics(fontinfo, codepoint, &advance, nullptr);
        GlyphValue glyph_value{};
        glyph_value.width = width;
        glyph_value.height = height;
        glyph_value.xoff = x_offset;
        glyph_value.yoff = y_offset;
        glyph_value.advance = static_cast<int>(advance * scale);
        glyph_value.bitmap.assign(bitmap, bitmap + width * height);
        stbtt_FreeBitmap(bitmap, nullptr);

        return glyph_cache.emplace(glyph_key, std::move(glyph_value)).first->second;
    }

    inline static auto layout_text(const ulx::str& text, const ulx::font& font) -> const TextLayout& {
        float scale = __uii::font::get_scale(font);
        ulx::u32 font_id = get_font_id(font.get_name());
        TextLayoutKey text_layout_key { text, font_id, font.get_size() };

        auto it = line_layout_cache.find(text_layout_key);
        if (it != line_layout_cache.end())
            return it->second;

        TextLayout layout;
        stbtt_fontinfo* fontinfo = __uii::font::load(font);

        layout.glyphs.reserve(text.size());

        ulx::i32 current_x = 0;
        ulx::i32 min_x = INT_MAX, max_x = INT_MIN;
        ulx::i32 min_y = INT_MAX, max_y = INT_MIN;

        for (ulx::size index = 0; index < text.size(); ++index) {
            ulx::u32 codepoint = static_cast<ulx::u32>(text[index]);
            const GlyphValue& glyph = get_glyph(font, codepoint, scale);

            TextLayout::GlyphPosition glyph_position;
            glyph_position.codepoint = codepoint;
            glyph_position.x = current_x + glyph.xoff;
            glyph_position.y = glyph.yoff;
            glyph_position.width = glyph.width;
            glyph_position.height = glyph.height;
            glyph_position.key = GlyphKey{ font_id, codepoint, font.get_size() };

            min_x = std::min(min_x, glyph_position.x);
            max_x = std::max(max_x, glyph_position.x + glyph_position.width);
            min_y = std::min(min_y, glyph_position.y);
            max_y = std::max(max_y, glyph_position.y + glyph_position.height);

            current_x += glyph.advance;
            if (index + 1 < text.size()) {
                ulx::u32 next = static_cast<ulx::u32>(text[index + 1]);
                current_x += static_cast<ulx::i32>(stbtt_GetCodepointKernAdvance(fontinfo, codepoint, next) * scale);
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
}
