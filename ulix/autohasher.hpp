#pragma once
#include <ulix/types.hpp>

namespace ulx {
    inline void hash_combine(u64& seed, u64 val) noexcept {
        seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T>
    u64 hash_value(const T& arg) noexcept(noexcept(std::hash<std::decay_t<T>>{}(arg))) {
        return std::hash<std::decay_t<T>>{}(arg);
    }

    template <typename... Args>
    inline auto make_hash(const Args&... args) -> ulx::u64 {
        u64 seed = 0;
        ((hash_combine(seed, hash_value(args))), ...);
        return static_cast<ulx::u64>(seed);
    }
}
