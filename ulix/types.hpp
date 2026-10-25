#pragma once
#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <array>
#include "__required_libraries/skarupke/flat_hash_map.hpp"
#include <chrono>

namespace ulx {
    /* chrono alias */
    namespace chr = std::chrono;
    namespace chrt {
        using ms = chr::milliseconds;
    }
    template<typename T> using time = chr::time_point<T>;
    using steady = chr::steady_clock;

    /* chrono functions */
    template<typename T>
    inline static constexpr auto ms_cast(const T& dur) -> chrt::ms {
        return chr::duration_cast<chrt::ms>(dur);
    }

    #ifndef ULIXRELEASE
        #define debug_msstart ulx::steady::now();
        #define debug_msinfo(last_steady) ulx::info("{}", ulx::ms_cast(ulx::steady::now() - last_steady))
    #endif
    

    /* type keyword alias */
    using i8 = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using f32 = float;
    using f64 = double;

    /* standard type alias */
    using str = std::string;
    using strvw = std::string_view;
    using wstr = std::wstring;
    template<typename T> using vec = std::vector<T>;
    template<typename T> using set = std::set<T>;
    template<typename T, ulx::u64 size> using arr = std::array<T, size>;
    template<typename A, typename B> using pair = std::pair<A, B>;
    using bytes = ulx::vec<u8>;

    /* extension type alias */
    template<typename K, typename V> using hashmap = ska::flat_hash_map<K, V>;
}
