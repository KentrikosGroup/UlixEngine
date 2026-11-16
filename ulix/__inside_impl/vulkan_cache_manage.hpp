#pragma once
#include <ulix/path.hpp>
#include <ulix/__inside_impl/log_utils.hpp>

namespace ulx::vcm {
    inline auto read_cache(const path& file_path) -> vec<char> {
        std::ifstream file = std::ifstream(file_path.str().cstr(), std::ios::ate | std::ios::binary);
        log::uassert(file.is_open(), "open vulkan cache file {} failed", file_path.str().cstr());
        
        u64 file_size = file.tellg();
        auto buffer = vec<char>(file_size);
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), file_size);
        
        file.close();
        return buffer;
    }

    inline void write_cache(const path& file_path, const vec<char>& buffer) {
        std::ofstream file = std::ofstream(file_path.str().cstr(), std::ios::binary);
        log::uassert(file.is_open(), "open vulkan cache file {} failed", file_path.str().cstr());
        file.write(buffer.data(), buffer.size());
        file.close();
    }

    inline void ensure_cache_exists(const path& file_path) {
        if (!file_path.exists()) write_cache(file_path, {});
    }
}