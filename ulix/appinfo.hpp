#pragma once

#include "file.hpp"
#include "types.hpp"
#include "requires.hpp"

namespace ulx {
    class appinfo {
        private:
            ulx::str application_id;
            ulx::file cache_file_path;
            ulx::u32 application_version;

        public:
            inline constexpr appinfo() = default;
            
            template<typename S, typename F> 
                requires ulx::expect<S, ulx::str> && ulx::expect<F, ulx::file>
            inline constexpr appinfo(S&& application_id, F&& cache_file_path):
                application_id(std::forward<S>(application_id)),
                cache_file_path(std::forward<F>(cache_file_path)),
                application_version(1) {}

            inline auto version(ulx::u32 major, ulx::u32 minor, ulx::u32 patch) -> appinfo& {
                application_version = (major << 22) | (minor << 12) | patch;
                return *this;
            }

        public:
            inline auto get_application_id() const -> const ulx::str& { return application_id; }
            inline auto get_application_version() const -> ulx::u32 { return application_version; }
            inline auto get_pipeline_cache_file_path() const -> const ulx::file& { return cache_file_path; }
    };
}
