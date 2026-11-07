#pragma once
#include "log.hpp"
#include "types.hpp"
#include "ulix/autohasher.hpp"
#include "ulix/requires.hpp"
#include <cstring>

namespace ulx {
    template<u64 size = 256>
    class stablestr {
        arr<char, size> str_array;

        public: template<typename t_strvw = strvw>
            requires expect<t_strvw, strvw>
        inline constexpr stablestr(t_strvw view = "") {
            strvw sview = strvw(view);
            
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(sview.size(), size - 1);
            std::memcpy(str_array.data(), sview.data(), len);
            str_array[len] = '\0';
        }

        // public: inline constexpr stablestr(const char* c_str) {
        //     ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
        //     auto len = std::min(std::strlen(c_str), size - 1);
        //     std::memcpy(str_array.data(), c_str, len);
        //     str_array[len] = '\0';
        // }

        // public: inline constexpr stablestr(char* c_str) {
        //     ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
        //     auto len = std::min(std::strlen(c_str), size - 1);
        //     std::memcpy(str_array.data(), c_str, len);
        //     str_array[len] = '\0';
        // }

        public: inline constexpr stablestr(const stablestr& other) {
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(other.len(), size - 1);
            std::memcpy(str_array.data(), other.cstr(), len);
            str_array[len] = '\0';
        }

        public: inline constexpr stablestr(stablestr&& other) {
            ulx::log::expect(size > 0, "stablestr.hpp: stablestr(): size: size must be at least 1");
            auto len = std::min(other.len(), size - 1);
            std::memcpy(str_array.data(), other.cstr(), len);
            str_array[len] = '\0';
            other.str_array[0] = '\0';
        }

        public: inline constexpr auto array() const -> const arr<char, size>& { return str_array; }
        public: inline constexpr auto array() -> arr<char, size>& { return str_array; }
        public: inline constexpr auto cstr() const -> const char* { return str_array.data(); }
        public: inline constexpr auto cstr() -> char* { return str_array.data(); }

        public: inline constexpr auto operator==(const stablestr& other) const -> bool {
            return std::strcmp(cstr(), other.cstr()) == 0;
        }

        public: inline constexpr auto operator=(const stablestr& other) -> stablestr& {
            if (this != &other)
                std::memcpy(str_array.data(), other.str_array.data(), size);

            return *this;
        }

        public: template<typename t_strvw = strvw>
            requires expect<t_strvw, strvw>
        inline constexpr auto operator=(t_strvw view) -> stablestr& {
            strvw sview = strvw(view); 
            
            auto len = std::min(sview.size(), size - 1);
            std::memcpy(str_array.data(), sview.data(), len);
            str_array[len] = '\0';
            return *this;
        }

        // public: inline constexpr auto operator=(const char* c_str) -> stablestr& {
        //     auto len = std::min(std::strlen(c_str), size - 1);
        //     std::memcpy(str_array.data(), c_str, len);
        //     str_array[len] = '\0';
        //     return *this;
        // }

        // public: inline constexpr auto operator=(char* c_str) -> stablestr& {
        //     auto len = std::min(std::strlen(c_str), size - 1);
        //     std::memcpy(str_array.data(), c_str, len);
        //     str_array[len] = '\0';
        //     return *this;
        // }

        public: inline constexpr auto operator!() -> bool {
            return strvw(str_array) == "";
        }

        public: template<typename t_strvw>
            requires expect<t_strvw, strvw>
        inline constexpr auto operator+=(t_strvw view) -> stablestr& {
            strvw sview = strvw(view);
            
            auto cur_len = std::strlen(cstr());
            auto remaining = size - cur_len - 1;
            auto copy_len = std::min(sview.size(), remaining);
            std::memcpy(str_array.data() + cur_len, sview.data(), copy_len);
            str_array[cur_len + copy_len] = '\0';
            return *this;
        }

        public: inline constexpr auto operator+=(const stablestr& str) -> stablestr& {
            auto cur_len = std::strlen(cstr());
            auto remaining = size - cur_len - 1;
            auto copy_len = std::min(str.len(), remaining);
            std::memcpy(str_array.data() + cur_len, str.cstr(), copy_len);
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

        public: template<typename t_strvw>
            requires expect<t_strvw, strvw>
        inline constexpr auto operator+(t_strvw view) const -> stablestr {
            strvw sview = strvw(view);
            
            stablestr result(*this);
            result += sview;
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

        // public: inline constexpr auto operator+(const char* cstr) const -> stablestr {
        //     stablestr result(*this);
        //     result += strvw(cstr);
        //     return result;
        // }

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

        public: template<typename T>
        static inline constexpr auto from(T value) -> stablestr {
            stablestr result;
            auto [ptr, ec] = std::to_chars(result.str_array.data(), result.str_array.data() + size - 1, value);
            *ptr = '\0';
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

namespace std {
    template<ulx::u64 size>
    struct hash<ulx::stablestr<size>> {
        size_t operator()(const ulx::stablestr<size>& s) const noexcept {
            return ulx::make_hash(ulx::strvw(s.cstr(), s.len()));
        }
    };
}
