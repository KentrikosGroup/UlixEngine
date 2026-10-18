#pragma once
#include "requires.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include <filesystem>
#include <fstream>

namespace ulx {
    class file {
        private:
            pathstr file_path;
    
        public:
            inline constexpr file() = default;

            template<typename _pathstr>
                requires ulx::expect<_pathstr, pathstr>
            inline constexpr file(_pathstr&& path) {
                file_path = std::forward<_pathstr>(path);
            }
            
            inline constexpr auto operator==(const file& other) const -> bool = default;
    
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
            inline auto path() const -> const pathstr& { return file_path; }
    };
}

