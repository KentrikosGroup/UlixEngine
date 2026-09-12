#pragma once

#include "types.hpp"
#include <filesystem>
#include <fstream>

namespace ulx {
    class file {
        private:
            ulx::str file_path;
    
        public:
            file() = default;
            file(const ulx::str file_path): file_path(file_path) {}
            bool operator==(const file&) const = default;
    
            inline auto read_string() const -> ulx::str {
                std::ifstream file_handle = std::ifstream(file_path);
                return ulx::str(std::istreambuf_iterator<char>(file_handle), std::istreambuf_iterator<char>());
            }
    
            inline auto read_bytes() const -> ulx::bytes {
                std::ifstream file_handle = std::ifstream(file_path, std::ios::binary);
                return ulx::bytes(std::istreambuf_iterator<char>(file_handle), std::istreambuf_iterator<char>());
            }
    
            inline auto exists() const -> bool { return std::filesystem::exists(file_path); }
            inline auto suffix() const -> ulx::str { return std::filesystem::path(file_path).extension().string(); }
    
        public:
            inline auto get_file_path() const -> const ulx::str& { return file_path; }
    };
}

template<> struct std::hash<ulx::file> {
    size_t operator()(const ulx::file& f) const noexcept {
        return std::hash<std::string>{}(f.get_file_path());
    }
};

template<> struct std::equal_to<ulx::file> {
    bool operator()(const ulx::file& a, const ulx::file& b) const noexcept {
        return a.get_file_path() == b.get_file_path();
    }
};
