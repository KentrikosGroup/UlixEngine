#pragma once

#include <concepts>
namespace ulx {
    template<typename T, typename E> concept expect = std::constructible_from<E, T>;
}