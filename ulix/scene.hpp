#pragma once
#include "log.hpp"
#include "requires.hpp"
#include "types.hpp"
#include "color.hpp"
#include "element.hpp"
#include "align.hpp"

namespace ulx {
    class scene {
        private:
            vec<element> elements;
            color background_color;
            u8 alignment = alignleft | aligntop;
            u32 paddin = 5;
            box bx = nonebox;
    
        public:
            scene() = default;
    
        public:
            template<typename _element>
                requires ulx::expect<_element, element>
            inline auto child(_element&& element) -> scene& {
                elements.push_back(std::forward<_element>(element));
                
                return *this;
            }
    
            inline auto back(const color& color) -> scene& {
                background_color = color;
                return *this;
            }
    
            inline auto align(u8 alignment) -> scene& {
                this->alignment = alignment;
                return *this;
            }
    
            inline auto pad(u32 padding) -> scene& {
                paddin = padding;
                return *this;
            }
    
            inline auto box(box objlayout) -> scene& {
                this->bx = objlayout;
                return *this;
            }

            inline auto layout(const layoutbox& layout) -> scene& {
                bx = layout.box;
                paddin = layout.padding;
                return *this;
            }
    
        public:
            inline auto get_objects() -> vec<element>& { return elements; }
            inline auto get_background_color() const -> const color& { return background_color; }
            inline auto get_alignment() const -> u8 { return alignment; }
            inline auto get_padding() const -> u32 { return paddin; }
            inline auto get_layout() const -> ulx::box { return bx; }
    };
}
