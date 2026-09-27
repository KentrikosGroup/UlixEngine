#pragma once

#include "types.hpp"

namespace ulx {
    enum winattr: ulx::u8 {
        resizable = 1 << 0,
        titled = 1 << 1,
        bordered = 1 << 2,
    
        normal = resizable | titled | bordered
    };
}
