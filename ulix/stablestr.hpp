#pragma once
#include "autohasher.hpp"
#include "log.hpp"
#include "types.hpp"
#include <cstring>

namespace ulx {
    template<u64 size = 256> 
    class stablestr {
        arr<char, size> str_array;

        public: inline constexpr stablestr(strvw view = "") {
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(view.size(), size - 1);
            std::memcpy(str_array.data(), view.data(), len);
            str_array[len] = '\0';
        }
        
        public: inline constexpr auto array() const -> const arr<char, size>& { return str_array; }
        public: inline constexpr auto cstr() const -> const char* { return str_array.data(); }

        public: inline constexpr auto operator<=>(const stablestr& other) const {
            return std::strcmp(cstr(), other.cstr()) <=> 0;
        }

        public: inline constexpr auto operator==(const stablestr& other) const -> bool {
            return std::strcmp(cstr(), other.cstr()) == 0;
        }

        public: inline constexpr auto operator=(const stablestr& other) -> stablestr& {
            if (this != &other)
                std::memcpy(str_array.data(), other.str_array.data(), size);

            return *this;
        }

        public: inline constexpr auto operator=(strvw view) -> stablestr& {
            auto len = std::min(view.size(), size - 1);
            std::memcpy(str_array.data(), view.data(), len);
            str_array[len] = '\0';
            return *this;
        }

        public: inline constexpr auto operator!() -> bool {
            return strvw(str_array) == "";
        }

        public: inline constexpr auto begin() const -> const char* { return str_array.data(); }
        public: inline constexpr auto end() const -> const char* { return str_array.data() + size; }
    };

    using pathstr = ulx::stablestr<260>;
}
