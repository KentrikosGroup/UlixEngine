#pragma once

#include <types.hpp>

namespace ulx {
    enum align: ulx::u8 {
        VCenter = 1 << 0,
        HCenter = 1 << 1,
        AlignTop = 1 << 2,
        AlignBottom = 1 << 3,
        AlignLeft = 1 << 4,
        AlignRight = 1 << 5,
        Center = VCenter | HCenter
    };
}