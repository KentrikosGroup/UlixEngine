#pragma once

#include "types.hpp"


namespace ulx {
    class txtobj {
        private:
            ulx::str text;
            
        public:
            txtobj() = default;
            inline constexpr txtobj(const ulx::str& text) : text(text) {}

        public:
            inline constexpr auto get_text() const -> const ulx::str& { return text; }            
    };
}