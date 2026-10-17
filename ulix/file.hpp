#pragma once
#include "requires.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include <filesystem>
#include <fstream>

namespace ulx {
    class file {
        private:
            stablestr<260> file_path;
    
        public:
            inline constexpr file() = default;
            
            template<typename S>
                requires expect<S, str>
            inline constexpr file(S&& file_path): file_path(std::forward<S>(file_path)) {}
            
            inline constexpr auto operator==(const file& other) const -> bool {
                return file_path == other.file_path;
            }
    
            inline auto read_string() const -> str {
                std::ifstream file_handle = std::ifstream(file_path.cstr());
                return str(std::istreambuf_iterator<char>(file_handle), std::istreambuf_iterator<char>());
            }
    
            inline auto read_bytes() const -> bytes {
                std::ifstream file_handle = std::ifstream(file_path.cstr(), std::ios::binary);
                return bytes(std::istreambuf_iterator<char>(file_handle), std::istreambuf_iterator<char>());
            }
    
            inline auto exists() const -> bool { return std::filesystem::exists(file_path.cstr()); }
            inline auto suffix() const -> str { return std::filesystem::path(file_path.cstr()).extension().string(); }
    
        public:
            inline auto path() const -> const stablestr<260>& { return file_path; }
    };
}

