#pragma once
#include <utility>
#include "types.hpp"

namespace ulx {
    template<typename T, typename E> concept expect = std::constructible_from<E, T>;
}
