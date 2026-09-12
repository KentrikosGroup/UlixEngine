#pragma once

#include <screen.hpp>
#include <rect.hpp>
#include <types.hpp>
#include <color.hpp>
#include <object.hpp>
#include <align.hpp>

namespace ulx {
    class scene {
        private:
            ulx::vec<object> objects;
            ulx::color background_color;
            ulx::u8 alignment = ulx::align::Center;
            ulx::u32 vpaddin = 5;
            ulx::u32 hpaddin = 5;
            ulx::layout objlayout = ulx::layout::none;
    
        public:
            scene() = default;
    
        public:
            inline auto child(object&& object) -> scene& {
                objects.push_back(object);
                
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
    
            inline auto vpadding(ulx::u32 padding) -> scene& {
                vpaddin = padding;
                return *this;
            }
    
            inline auto hpadding(ulx::u32 padding) -> scene& {
                hpaddin = padding;
                return *this;
            }
    
            inline auto padding(ulx::u32 padding) -> scene& {
                vpaddin = padding;
                hpaddin = padding;
                return *this;
            }
    
            inline auto layout(ulx::layout objlayout) -> scene& {
                this->objlayout = objlayout;
                return *this;
            }
    
        public:
            inline auto get_objects() const -> const ulx::vec<object>& { return objects; }
            inline auto get_background_color() const -> const ulx::color& { return background_color; }
            inline auto get_alignment() const -> ulx::u8 { return alignment; }
            inline auto get_vpadding() const -> ulx::u32 { return vpaddin; }
            inline auto get_hpadding() const -> ulx::u32 { return hpaddin; }
            inline auto get_layout() const -> ulx::layout { return objlayout; }
    };
}
