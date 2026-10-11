#pragma once
#include "file.hpp"
#include "stablestr.hpp"
#include "types.hpp"
#include "requires.hpp"

namespace ulx {
    class appinfo {
        private:
            stablestr<> app_id;
            
            file cache_file_path;
            u32 application_version;
            file icon_file_path;

        public:
            inline constexpr appinfo() = default;
            
            template<typename S, typename F> 
                requires expect<S, str> && expect<F, file>
            inline constexpr appinfo(S&& application_id, F&& cache_file_path):
                app_id(std::forward<S>(application_id)),
                cache_file_path(std::forward<F>(cache_file_path)),
                application_version(1),
                icon_file_path("") {}

            inline auto version(u32 major, u32 minor, u32 patch) -> appinfo& {
                application_version = (major << 22) | (minor << 12) | patch;
                return *this;
            }

            template<typename F>
                requires expect<F, file>
            inline auto icon(F&& icon_file_path) -> appinfo& {
                this->icon_file_path = std::forward<F>(icon_file_path);
                return *this;
            }

        public:
            inline auto get_application_id() const -> const stablestr<>& { return app_id; }
            inline auto get_application_version() const -> u32 { return application_version; }
            inline auto get_pipeline_cache_file_path() const -> const file& { return cache_file_path; }
            inline auto get_icon_file_path() const -> const file& { return icon_file_path; }
    };
}
