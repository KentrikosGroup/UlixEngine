#pragma once
#include "path.hpp"
#include "requires.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include <utility>

namespace ulx { 
    class appinfo {
        stablestr<> appid;
        path cache_dir;
        u32 application_version = 1;
        path icon_file_path;
    
        public: inline constexpr appinfo() = default;

        public: template<typename _stablestr, typename _path>
            requires ulx::expect<_stablestr, stablestr<>> && ulx::expect<_path, path>
            inline constexpr appinfo(_stablestr&& appid, _path&& cache) {
            
            this->appid = std::forward<_stablestr>(appid);
            this->cache_dir = std::forward<_path>(cache);
        }
    
        public: inline constexpr auto version(u32 major, u32 minor, u32 patch) -> appinfo& {
            application_version = (major << 22) | (minor << 12) | patch;
            return *this;
        }
    
        public: template<typename _path>
            requires ulx::expect<_path, path>
        inline constexpr auto icon(_path&& icon) -> appinfo& {    
            this->icon_file_path = std::forward<_path>(icon_file_path);
            return *this;
        }

        public: inline constexpr auto operator=(const appinfo& other) -> appinfo& = default;
    
        public: inline constexpr auto get_application_id() const -> const stablestr<>& { return appid; }
        public: inline constexpr auto get_application_version() const -> u32 { return application_version; }
        public: inline constexpr auto get_cache_dir() const -> const path& { return cache_dir; }
        public: inline constexpr auto get_icon_file_path() const -> const path& { return icon_file_path; }
    };
}
