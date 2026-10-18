#pragma once
#include "file.hpp"
#include "requires.hpp"
#include "stablestr.hpp"
#include "types.hpp"

namespace ulx { 
    class appinfo {
        stablestr<> appid;
        file cache_file;
        u32 application_version = 1;
        file icon_file_path;
    
        public: inline constexpr appinfo() = default;

        public: template<typename _str, typename _file>
            requires ulx::expect<_str, str> && ulx::expect<_file, file>
            inline constexpr appinfo(_str&& appid, _file&& cache) {
            
            this->appid = appid;
            this->cache_file = cache;
        }
    
        public: inline constexpr auto version(u32 major, u32 minor, u32 patch) -> appinfo& {
            application_version = (major << 22) | (minor << 12) | patch;
            return *this;
        }
    
        public: template<typename _file>
            requires ulx::expect<_file, file>
        inline constexpr auto icon(_file&& icon) -> appinfo& {    
            this->icon_file_path = std::forward<_file>(icon_file_path);
            return *this;
        }

        public: inline constexpr auto operator=(const appinfo& other) -> appinfo& = default;
    
        public: inline constexpr auto get_application_id() const -> const stablestr<>& { return appid; }
        public: inline constexpr auto get_application_version() const -> u32 { return application_version; }
        public: inline constexpr auto get_pipeline_cache_file_path() const -> const file& { return cache_file; }
        public: inline constexpr auto get_icon_file_path() const -> const file& { return icon_file_path; }
    };
}
