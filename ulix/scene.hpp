#pragma once

#include "requires.hpp"
#include "screen.hpp"
#include "rect.hpp"
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
            ulx::layout layout_ = ulx::layout::nonebox;
    
        public:
            scene() = default;
    
        public:
            template<typename O>
                requires ulx::expect<O, ulx::object>
            inline auto child(O&& object) -> scene& {
                objects.push_back(std::forward<O>(object));
                
                return *this;
            }
    
            inline auto background(const ulx::color& color) -> scene& {
                background_color = color;
                return *this;
            }
    
            inline auto align(ulx::u8 alignment) -> scene& {
                this->alignment = alignment;
                return *this;
            }
    
            inline auto padding(ulx::u32 padding) -> scene& {
                paddin = padding;
                return *this;
            }
    
            inline auto layout(ulx::layout objlayout) -> scene& {
                this->layout_ = objlayout;
                return *this;
            }
    
        public:
            inline auto get_objects() const -> const ulx::vec<object>& { return objects; }
            inline auto get_background_color() const -> const ulx::color& { return background_color; }
            inline auto get_alignment() const -> ulx::u8 { return alignment; }
            inline auto get_padding() const -> ulx::u32 { return paddin; }
            inline auto get_layout() const -> ulx::layout { return layout_; }
    };
}
