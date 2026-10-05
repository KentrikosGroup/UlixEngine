#pragma once

#include "requires.hpp"
#include "screen.hpp"
#include "types.hpp"
#include "color.hpp"
#include "object.hpp"
#include "align.hpp"

namespace ulx {
    class scene {
        private:
            ulx::vec<object> objects;
            ulx::color background_color;
            ulx::u8 alignment = ulx::alignleft | ulx::aligntop;
            ulx::u32 paddin = 5;
            ulx::box bx = ulx::box::nonebox;
    
        public:
            scene() = default;
    
        public:
            template<typename O>
                requires ulx::expect<O, ulx::object>
            inline auto child(O&& object) -> scene& {
                objects.push_back(std::forward<O>(object));
                
                return *this;
            }
    
            inline auto back(const ulx::color& color) -> scene& {
                background_color = color;
                return *this;
            }
    
            inline auto align(ulx::u8 alignment) -> scene& {
                this->alignment = alignment;
                return *this;
            }
    
            inline auto pad(ulx::u32 padding) -> scene& {
                paddin = padding;
                return *this;
            }
    
            inline auto box(ulx::box objlayout) -> scene& {
                this->bx = objlayout;
                return *this;
            }

            inline auto layout(const layoutbox& layout) -> scene& {
                bx = layout.box;
                paddin = layout.padding;
                return *this;
            }
    
        public:
            inline auto get_objects() -> ulx::vec<object>& { return objects; }
            inline auto get_background_color() const -> const ulx::color& { return background_color; }
            inline auto get_alignment() const -> ulx::u8 { return alignment; }
            inline auto get_padding() const -> ulx::u32 { return paddin; }
            inline auto get_layout() const -> ulx::box { return bx; }
    };
}
