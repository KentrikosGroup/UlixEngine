#pragma once

#include "types.hpp"

namespace ulx {
    enum winattr: ulx::u8 {
        resizableattr = 1 << 0,
        titledattr = 1 << 1,
        borderedattr = 1 << 2,
    
        normalattr = resizableattr | titledattr | borderedattr
    };
}
