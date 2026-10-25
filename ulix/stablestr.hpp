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

        public: inline constexpr stablestr(const char* c_str) {
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(std::strlen(c_str), size - 1);
            std::memcpy(str_array.data(), c_str, len);
            str_array[len] = '\0';
        }

        public: inline constexpr stablestr(char* c_str) {
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(std::strlen(c_str), size - 1);
            std::memcpy(str_array.data(), c_str, len);
            str_array[len] = '\0';
        }

        public: inline constexpr stablestr(const stablestr& other) {
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(other.len(), size - 1);
            std::memcpy(str_array.data(), other.cstr(), len);
            str_array[len] = '\0';
        }
        
        public: inline constexpr stablestr(stablestr&& other) noexcept {
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(other.len(), size - 1);
            std::memcpy(str_array.data(), other.cstr(), len);
            str_array[len] = '\0';
            other.str_array[0] = '\0';
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

        public: inline constexpr auto operator=(const char* c_str) -> stablestr& {
            auto len = std::min(std::strlen(c_str), size - 1);
            std::memcpy(str_array.data(), c_str, len);
            str_array[len] = '\0';
            return *this;
        }

        public: inline constexpr auto operator=(char* c_str) -> stablestr& {
            auto len = std::min(std::strlen(c_str), size - 1);
            std::memcpy(str_array.data(), c_str, len);
            str_array[len] = '\0';
            return *this;
        }

        public: inline constexpr auto operator!() -> bool {
            return strvw(str_array) == "";
        }

        public: inline constexpr auto operator+=(strvw view) -> stablestr& {
            auto cur_len = std::strlen(cstr());
            auto remaining = size - cur_len - 1;
            auto copy_len = std::min(view.size(), remaining);
            std::memcpy(str_array.data() + cur_len, view.data(), copy_len);
            str_array[cur_len + copy_len] = '\0';
            return *this;
        }

        public: inline constexpr auto operator+=(char ch) -> stablestr& {
            auto cur_len = std::strlen(cstr());
            if (cur_len < size - 1) {
                str_array[cur_len] = ch;
                str_array[cur_len + 1] = '\0';
            }
            return *this;
        }
        
        public: inline constexpr auto operator+(strvw view) const -> stablestr {
            stablestr result(*this);
            result += view;
            return result;
        }

        public: inline constexpr auto operator+(char ch) const -> stablestr {
            stablestr result(*this);
            result += ch;
            return result;
        }
        
        public: inline constexpr auto operator+(const stablestr& other) const -> stablestr {
            return *this + other.cstr();
        }

        public: friend inline constexpr auto operator+(char lhs, const stablestr& rhs) -> stablestr {
            stablestr result(strvw(&lhs, 1));
            result += rhs.cstr();
            return result;
        }

        public: friend inline constexpr auto operator+(const char* lhs, const stablestr& rhs) -> stablestr {
            stablestr result(lhs);
            result += rhs.cstr();
            return result;
        }

        public: inline constexpr auto begin() const -> const char* { return str_array.data(); }
        public: inline constexpr auto end() const -> const char* { return str_array.data() + size; }
        public: inline constexpr auto len() const -> u64 { return std::strlen(cstr()); }
        public: inline constexpr auto copy() const -> stablestr { return stablestr(*this); }
    };

    using u8char = stablestr<4>;
    using pathstr = stablestr<260>;
}
