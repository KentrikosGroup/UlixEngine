#pragma once
#include "autohasher.hpp"
#include "log.hpp"
#include "types.hpp"
#include <cstring>

namespace ulx {
    template<u64 size = 256> class stablestr {
        private:
            arr<char, size> str_array;

        public:
            inline constexpr stablestr(strvw view = "") {
                ulx::log::expect(size, "stablestr.hpp: stablestr(): size: size must be at least 1");
                auto len = std::min(view.size(), size - 1);
                std::memcpy(str_array.data(), view.data(), len);
                str_array[len] = '\0';
            }
            
            inline constexpr auto array() const -> const arr<char, size>& { return str_array; }
            inline constexpr auto cstr() const -> const char* { return str_array.data(); }

            inline constexpr auto operator<=>(const stablestr& other) const {
                return std::strcmp(cstr(), other.cstr()) <=> 0;
            }

            inline constexpr bool operator==(const stablestr& other) const {
                return std::strcmp(cstr(), other.cstr()) == 0;
            }

            inline constexpr auto begin() const -> const char* { return str_array.data(); }
            inline constexpr auto end() const -> const char* { return str_array.data() + size; }
    };

    using pathstr = ulx::stablestr<260>;
}
