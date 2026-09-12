#pragma once

#include <log.hpp>

#ifndef ULIXRELEASEAPPLICATION
    #define TODO(x) ([]() -> decltype(auto) { ULogger::ulixwarn("unfinished todo: {}", (x)); return (x); })()
    #define BUG(x) ([]() -> decltype(auto) { ULogger::ulixerr("unresolved bug: {}", (x)); })()
    #define DEBUG(x) ([]() -> decltype(auto) { ULogger::ulixwarn("debug index: {}", (x)); })()
#else
    #define TODO(x) (x)
    #define BUG(x) ([]() -> decltype(auto) { ULogger::ulixerr("unresolved bug: {}", (x)); })()
    #define DEBUG(x) ([]() -> decltype(auto) { })()
#endif
