#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <ulix/types.hpp>
#include <format>
#include <print>
#include <windows.h>

namespace ulx::log {
    template<typename... t_args>
    [[noreturn]] inline static auto uerror(strvw info, t_args&&... args) {
        auto fmted = std::vformat(info, std::make_format_args(args...));
        std::println("ulix error: {}", fmted);
        ExitProcess(-1);
    }
    
    template<typename... t_args>
    inline static auto uassert(bool condition, strvw info, t_args&&... args) {
        if (!condition) uerror(info, args...); 
    }

    template<typename... t_args>
    inline static auto uwarn(strvw info, t_args&&... args) {
        auto fmted = std::vformat(info, std::make_format_args(args...));
        std::println("ulix warning: {}", fmted);
    }
}
