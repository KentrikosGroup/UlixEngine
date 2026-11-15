#pragma once

#include "types.hpp"
#include "ulix/log.hpp"
#include <cstddef>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ulx::wfn {
    inline UINT screen_physical_width() {
        DISPLAY_DEVICEW display_device = { sizeof(display_device) };
        EnumDisplayDevicesW(nullptr, 0, &display_device, 0);

        DEVMODEW devmode = {};
        devmode.dmSize = sizeof(devmode);
        EnumDisplaySettingsW(display_device.DeviceName, ENUM_CURRENT_SETTINGS, &devmode);
        return devmode.dmPelsWidth;
    }

    inline UINT screen_physical_height() {
        DISPLAY_DEVICEW display_device = { sizeof(display_device) };
        EnumDisplayDevicesW(nullptr, 0, &display_device, 0);

        DEVMODEW devmode = {};
        devmode.dmSize = sizeof(devmode);
        EnumDisplaySettingsW(display_device.DeviceName, ENUM_CURRENT_SETTINGS, &devmode);
        return devmode.dmPelsHeight;
    }

    inline static auto __init = []() -> std::nullptr_t {
        ulx::log::expect(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2), 
            "failed to set DPI awareness, error code: {}", GetLastError());
        return nullptr;
    }();
    
    inline static const UINT system_dpi = GetDpiForSystem();
    inline static const HINSTANCE hinstance = GetModuleHandleW(nullptr);
    inline static const f32 system_scale = f32(system_dpi) / 96.0f;
    inline static constexpr f32 base_dpi = 96.0f;
    
    inline constexpr auto logical(f32 physical_pixels) -> ulx::f32 {
        return physical_pixels * (base_dpi / system_dpi);
    }
    
    inline constexpr auto physical(f32 logical_pixels) -> ulx::f32 {
        return logical_pixels * (system_dpi / base_dpi);
    }

    inline constexpr auto make_int_resource(WORD id) -> LPCWSTR {
        return reinterpret_cast<LPCWSTR>(ULONG_PTR(id));
    }
}
