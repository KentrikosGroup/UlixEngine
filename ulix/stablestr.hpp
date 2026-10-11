#pragma once
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

template<ulx::u64 size> struct std::hash<ulx::stablestr<size>> {
    ulx::u64 operator()(const ulx::stablestr<size>& s) const noexcept {
        ulx::u64 h = 14695981039346656037ULL;
        for (const char* p = s.cstr(); *p; ++p) {
            h ^= static_cast<ulx::u64>(*p);
            h *= 1099511628211ULL;
        }
        return h;
    }
};

template<ulx::u64 size> struct std::equal_to<ulx::stablestr<size>> {
    bool operator()(const ulx::stablestr<size>& lhs, const ulx::stablestr<size>& rhs) const {
        return lhs == rhs;
    }
};