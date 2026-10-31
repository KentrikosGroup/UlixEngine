#pragma once
#pragma once
#include "requires.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include <filesystem>
#include <fstream>

namespace ulx {
    class path {
        pathstr m_path;
    
        public: inline constexpr path() = default;

        public: template<typename _pathstr>
                requires ulx::expect<_pathstr, pathstr>
        inline constexpr path(_pathstr&& path): m_path(std::forward<_pathstr>(path)) { correct_path(); }
        public: inline constexpr path(std::filesystem::path&& path): m_path(std::move(path.string())) { correct_path(); }
        public: inline constexpr path(const std::filesystem::path& path): m_path(path.string()) { correct_path(); }
        public: inline constexpr auto operator==(const path& other) const -> bool = default;
    
        public: inline constexpr auto read_string() const -> str {
            std::ifstream file_handle = std::ifstream(m_path.cstr());
            return ulx::str(std::istreambuf_iterator<char>(file_handle), std::istreambuf_iterator<char>());
        }

        public: inline constexpr auto read_bytes() const -> bytes {
            std::ifstream file_handle = std::ifstream(m_path.cstr(), std::ios::binary);
            return bytes(std::istreambuf_iterator<char>(file_handle), std::istreambuf_iterator<char>());
        }

        public: inline constexpr auto exists() const -> bool { return std::filesystem::exists(m_path.cstr()); }
        public: inline constexpr auto suffix() const -> str { return std::filesystem::path(m_path.cstr()).extension().string(); }
        public: inline constexpr auto str() const -> const pathstr& { return m_path; }

        private: inline constexpr auto withps(const pathstr& subpath) const -> path {
            const char* cur = m_path.cstr();
            const char* sub = subpath.cstr();
            
            if (!cur || !sub || *sub == '\0') return *this;       
            while (*sub == '/' || *sub == '\\') ++sub;
            
            size_t cur_len = strlen(cur);
            bool need_sep = (cur_len > 0 && cur[cur_len - 1] != '\\');
            pathstr result_str = m_path;
            if (need_sep) result_str += '\\';
            result_str += sub;
            
            return path(result_str);
        }

        public: template<typename... Args>
        inline constexpr auto with(Args&&... args) const -> path {
            path result = *this;
            ((result = result.withps(std::forward<Args>(args))), ...);
            return result;
        }
            
        private: inline constexpr auto correct_path() -> void {
            char* start = m_path.cstr();
            char* s = start;
            if (!s) return;
                
            while (*s && isspace(*s)) ++s;
            if (*s == '\0') { start[0] = '\0'; return; }
            
            char* end = s + strlen(s) - 1;
            while (end > s && isspace(*end)) --end;
            
            size_t o = 0;
            bool net = (s[0] == '/' || s[0] == '\\') && (s[1] == '/' || s[1] == '\\');
            if (net) {
                start[o++] = '\\'; start[o++] = '\\';
                s += 2;
                while (s <= end && (*s == '/' || *s == '\\')) ++s;
            }
            
            while (s <= end) {
                if (*s == '/' || *s == '\\') {
                    start[o++] = '\\';
                    ++s;
                    while (s <= end && (*s == '/' || *s == '\\')) ++s;
                } else {
                    start[o++] = *s++;
                }
            }
            
            if (o > 0 && start[o-1] == '\\') {
                bool keep = (o == 3 && start[1] == ':') || (net && o == 2);
                if (!keep) o--;
            }
            
            start[o] = '\0';
        }

        public: inline constexpr auto parent() const -> path {
            const char* cur = m_path.cstr();
            if (!cur || *cur == '\0') return *this;
            
            size_t len = m_path.len();
            size_t pos = len;
            while (pos > 0) {
                --pos;
                if (cur[pos] == '\\') break;
            }
            
            if (pos == 0 && cur[pos] != '\\') return path();
            if (pos == 1 && cur[0] == '\\' && cur[1] == '\\') {
                size_t pos2 = len;
                while (pos2 > 2) {
                    --pos2;
                    if (cur[pos2] == '\\') break;
                }
                if (pos2 > 2) pos = pos2;
            }
            
            pathstr result_str = m_path;
            char* s = result_str.cstr();
            s[pos] = '\0';
            
            return path(result_str);
        }
    };
}

