#pragma once

#include "size.hpp"
#include "winfunc.hpp"
#include "types.hpp"

namespace ulx::screen {
    inline static auto size() -> ulx::size { 
        return { wfn::logical(wfn::screen_physical_width()), 
                 wfn::logical(wfn::screen_physical_height()) }; 
    }
    inline static auto width() -> ulx::f32 { return size().width(); }
    inline static auto height() -> ulx::f32 { return size().height(); }
    inline static auto dpi() -> ulx::u32 { return wfn::system_dpi; }
}
