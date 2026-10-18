#pragma once
#include "types.hpp"
#include <cstdlib>

namespace ulx {
    template<typename ptrtype>
    class autotmp {
        ptrtype tmp;
        u64 sz = 0;

        public: inline constexpr autotmp() = default;
        public: template<typename _type>
            inline constexpr autotmp(_type&& tmp) {
            this->tmp = std::forward<_type>(tmp);
        }

        public: inline constexpr auto temp() -> ptrtype* {
            return &tmp;
        }

        public: inline constexpr auto size() -> u64* {
            return &sz;
        }

        public: inline constexpr ~autotmp() {
            if (tmp) free(tmp);
            sz = 0;
        }

        public: inline constexpr auto operator!() {
            return tmp == nullptr;
        }
    };
}