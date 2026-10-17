#pragma once

#include "size.hpp"
#include "winfunc.hpp"
#include "types.hpp"
#include "log.hpp"

namespace ulx::screen {
    inline static ulx::f32 _screen_width, _screen_height;

    inline static auto size() -> ulx::size { return {_screen_width, _screen_height}; }
    inline static auto width() -> ulx::f32 { return _screen_width; }
    inline static auto height() -> ulx::f32 { return _screen_height; }
    inline static auto dpi() -> ulx::u32 { return ulx::wfn::system_dpi; }

    struct _init_s {
        inline constexpr _init_s() {
            ulx::log::expect(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2), 
                "failed to set DPI awareness, error code: {}", GetLastError());

            _screen_width = wfn::logical(wfn::screen_width);
            _screen_height = wfn::logical(wfn::screen_physical_height());
        }
    } inline static _init;
}
