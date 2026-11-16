#pragma once
#include <ulix/size.hpp>
#include <ulix/winfunc.hpp>
#include <ulix/types.hpp>

namespace ulx::screen {
    inline static auto size() -> ulx::size { 
        return { wfn::logical(wfn::screen_physical_width()), 
                 wfn::logical(wfn::screen_physical_height()) }; 
    }
    inline static auto w() -> ulx::f32 { return size().w(); }
    inline static auto h() -> ulx::f32 { return size().h(); }
    inline static auto dpi() -> ulx::u32 { return wfn::system_dpi; }
}
