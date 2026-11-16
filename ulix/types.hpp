#pragma once
#include <cstdint>
#include <expected>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <array>
#include <chrono>
#include <ulix/__required_libraries/skarupke/flat_hash_map.hpp>

namespace ulx {
    // chrono type alias
    namespace chr = std::chrono;
    namespace chrt {
        using ms = chr::milliseconds;
    }
    template<typename T> using time = chr::time_point<T>;
    using steady = chr::steady_clock;

    // chrono functions
    template<typename T>
    inline static constexpr auto ms_cast(const T& dur) -> chrt::ms {
        return chr::duration_cast<chrt::ms>(dur);
    }

    template<typename t_int>
    inline static consteval auto enumn(t_int v) {
        return t_int(1) << v;
    }

    // keyword type alias 
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

    // standard type alias
    using str = std::string;
    using strvw = std::string_view;
    using wstr = std::wstring;
    template<typename t_any> using vec = std::vector<t_any>;
    template<typename t_any> using set = std::set<t_any>;
    template<typename t_any, ulx::u64 size> using arr = std::array<t_any, size>;
    template<typename t_any1, typename t_any2> using pair = std::pair<t_any1, t_any2>;
    template<typename t_any> using opt = std::optional<t_any>;
    template<typename t_any, typename t_errany> using exped = std::expected<t_any, t_errany>;
    template<typename t_errany> using unexped = std::unexpected<t_errany>;
    using bytes = ulx::vec<u8>;

    // extension type alias
    template<typename t_keyany, typename t_valany> using hashmap = ska::flat_hash_map<t_keyany, t_valany>;
}
