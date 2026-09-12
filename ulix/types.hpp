#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <array>

namespace ulx {
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
    using size = std::size_t;
    using str = std::string;
    using strvw = std::string_view;
    using wstr = std::wstring;
    using bytes = std::vector<u8>;
    template<typename T> using vec = std::vector<T>;
    template<typename T> using set = std::set<T>;
    template<typename T, ulx::size array_size> using arr = std::array<T, array_size>;
    template<typename K, typename V> using hashmap = std::unordered_map<K, V>;
    template<typename A, typename B> using pair = std::pair<A, B>;

    #ifdef __clang__
        using i128 = __int128_t;
        using u128 = __uint128_t;
    #endif
}
