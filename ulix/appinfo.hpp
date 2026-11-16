#pragma once
#include <ulix/path.hpp>
#include <ulix/requires.hpp>
#include <ulix/stablestr.hpp>

namespace ulx {
    class appinfo {
        stablestr<> m_appid;
        path        m_cachedir;
        path        m_iconfile;

        public: inline constexpr appinfo() = default;

        public: template<typename _stablestr, typename _path>
            requires expect<_stablestr, stablestr<>> && expect<_path, path>
            inline constexpr appinfo(_stablestr&& appid, _path&& cache) {

            this->m_appid = std::forward<_stablestr>(appid);
            this->m_cachedir = std::forward<_path>(cache);
        }

        public: template<typename _path>
            requires expect<_path, path>
        inline constexpr auto icon(_path&& icon) -> appinfo& {
            this->m_iconfile = std::forward<_path>(m_iconfile);
            return *this;
        }

        public: inline constexpr auto operator=(const appinfo& other) -> appinfo& = default;

        public: inline constexpr auto get_appid() const -> const stablestr<>& { return m_appid; }
        public: inline constexpr auto get_cachedir() const -> const path& { return m_cachedir; }
        public: inline constexpr auto get_iconfile() const -> const path& { return m_iconfile; }
    };
}
