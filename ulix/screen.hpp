#pragma once

#include "winfunc.hpp"
#include "rect.hpp"
#include "types.hpp"

namespace ulx {
    class screen {
        public:
            inline static auto get_rect() -> ulx::rect {
                return ulx::rect(0, 0,
                    ulx::wfn::logical_cast(GetSystemMetricsForDpi(SM_CXSCREEN, ulx::wfn::system_dpi)),
                    ulx::wfn::logical_cast(GetSystemMetricsForDpi(SM_CYSCREEN, ulx::wfn::system_dpi)));
            }
            
            inline static auto get_dpi() -> ulx::u32 {
                return ulx::wfn::system_dpi;
            }
    };
}
