#pragma once
#include <ulix/requires.hpp>
#include <ulix/types.hpp>
#include <ulix/color.hpp>
#include <ulix/element.hpp>
#include <ulix/align.hpp>

namespace ulx {
    class scene {
        vec<element> m_elements;
        color        m_back_color;
        u8           m_align   = aligndef;
        u32          m_padding = 5;
        box          m_box     = nonbox;
    
        public: scene() = default;
    
        public: template<typename _element>
                requires ulx::expect<_element, element>
        inline auto child(_element&& element) -> scene& {
            m_elements.push_back(std::forward<_element>(element));
            
            return *this;
        }

        public: inline auto back(const color& color) -> scene& {
            m_back_color = color;
            return *this;
        }

        public: inline auto align(u8 align) -> scene& {
            this->m_align = align;
            return *this;
        }

        public: inline auto pad(u32 padding) -> scene& {
            m_padding = padding;
            return *this;
        }

        public: inline auto box(box box) -> scene& {
            this->m_box = box;
            return *this;
        }

        public: inline auto get_elements() -> vec<element>& { return m_elements; }
        public: inline auto get_back_color() const -> const color& { return m_back_color; }
        public: inline auto get_alignment() const -> u8 { return m_align; }
        public: inline auto get_padding() const -> u32 { return m_padding; }
        public: inline auto get_layout() const -> ulx::box { return m_box; }
    };
}
