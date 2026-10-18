#pragma once
#include <utility>
#include "types.hpp"

namespace ulx {
    template<typename T, typename E> concept expect = std::constructible_from<E, T>;
    template<typename T> using identity = std::type_identity_t<T>;
}
