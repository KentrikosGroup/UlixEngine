#pragma once
#include <mutex>
#include <ulix/callback.hpp>

namespace ulx {
    template<typename t_anyfn>
    inline static auto guard(std::mutex& mtx, const t_anyfn& fn) -> decltype(auto) {
        auto lock = std::lock_guard(mtx);
        return fn();
    }
}