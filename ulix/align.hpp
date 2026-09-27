#pragma once

#include "types.hpp"

namespace ulx {
    enum align: ulx::u8 {
        vcenter = 1 << 0,
        hcenter = 1 << 1,
        aligntop = 1 << 2,
        alignbottom = 1 << 3,
        alignleft = 1 << 4,
        alignright = 1 << 5,
        center = vcenter | hcenter
    };
}