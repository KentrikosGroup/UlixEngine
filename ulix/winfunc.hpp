#pragma once

#include <types.hpp>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ulx::wfn {
    inline static const UINT system_dpi = GetDpiForSystem();
    inline static const HINSTANCE hinstance = GetModuleHandleW(nullptr);

    inline constexpr auto physical_cast(ulx::f32 logical_pixels, UINT dpi = system_dpi) -> ulx::f32 {
        return logical_pixels * (static_cast<ulx::f32>(dpi) / 96.0f);
    }

    inline constexpr auto logical_cast(ulx::f32 physical_pixels, UINT dpi = system_dpi) -> ulx::f32 {
        return physical_pixels * (96.0f / static_cast<ulx::f32>(dpi));
    }

    inline constexpr auto make_int_resource(WORD id) -> LPCWSTR {
        return reinterpret_cast<LPCWSTR>(static_cast<ULONG_PTR>(id));
    }
}
