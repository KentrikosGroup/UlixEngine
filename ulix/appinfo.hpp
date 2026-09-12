#pragma once

#include <file.hpp>
#include <types.hpp>

namespace ulx {
    class appinfo {
        private:
            ulx::str application_id;
            ulx::file cache_file_path;
            ulx::u32 application_version;
            bool enabled_vulkan_debug;
    
        public:
            appinfo() = default;
            inline constexpr appinfo(const ulx::str& application_id, const ulx::file& cache_file_path):
                application_id(application_id),
                cache_file_path(cache_file_path),
                application_version(1),
                enabled_vulkan_debug(true)
            {}
    
            inline auto version(ulx::u32 major, ulx::u32 minor, ulx::u32 patch) -> appinfo& {
                application_version = (major << 22) | (minor << 12) | patch;
    
                return *this;
            }
    
            inline auto vkdbg(bool enabled = true) -> appinfo& {
                enabled_vulkan_debug = enabled;
    
                return *this;
            }
    
        public:
            inline auto get_application_id() const -> const ulx::str& { return application_id; }
            inline auto get_application_version() const -> ulx::u32 { return application_version; }
            inline auto get_pipeline_cache_file_path() const -> const ulx::file& { return cache_file_path; }
            inline auto get_enabled_vulkan_debug() const -> bool { return enabled_vulkan_debug; }
    };
}
