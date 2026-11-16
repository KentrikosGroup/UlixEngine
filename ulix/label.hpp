#pragma once
#include <ulix/element.hpp>
#include <ulix/__inside_impl/font_glyph_getter.hpp>

namespace ulx {
    class label: public element {
        public: inline constexpr label() = default;
        public: inline constexpr label(strvw id, const str& txt, const font& font = {}, const color& color = {255, 255, 255}) {
            auto metrics = fgg::line_metrics(font);
            f32 height = wfn::logical(metrics.template get<2>());

            size({0, height});
            align(alignleft | aligntop);
            box(horbox).pad(0);

            for (u64 i = 0; i < txt.size(); ) {
                u32 cp = fgg::decode_utf8(txt, i);
                u8char glyph_str;

                if (cp < 0x80) {
                    glyph_str += cp;
                } else if (cp < 0x800) {
                    glyph_str += 0xC0 | (cp >> 6);
                    glyph_str += 0x80 | (cp & 0x3F);
                } else {
                    glyph_str += 0xE0 | (cp >> 12);
                    glyph_str += 0x80 | ((cp >> 6) & 0x3F);
                    glyph_str += 0x80 | (cp & 0x3F);
                }

                auto c = element(glyph_str.cstr())
                    .texture(glyph_str, font)
                    .size(ulx::size::autos())
                    .border(0).corner(0).back(color::empty());
                c.data(glm::vec4{color.r1(), color.g1(), color.b1(), color.a1()});
                child(c);
            }
        }
    };
}