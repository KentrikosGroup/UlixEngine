#pragma once
#include "types.hpp"

namespace ulx {
    enum align: ulx::u8 {
        aligntop    = enumn(0),
        alignbottom = enumn(1),
        alignleft   = enumn(2),
        alignright  = enumn(3),
        aligndef = aligntop | alignleft
    };
}