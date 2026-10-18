#pragma once
#include "log.hpp"
#include "types.hpp"
#include <functional>
#include <type_traits>
#include <utility>

inline void hash_combine(std::size_t& seed, std::size_t val) {
    seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T, typename... As>
inline static consteval auto member_count_impl() -> ulx::u64 {
    if constexpr (requires { T{As{}...}; })
        return member_count_impl<T, As..., int>();
    else return sizeof...(As) - 1;
}

template<typename T>
inline static consteval auto member_count() -> ulx::u64 {
    return member_count_impl<T>();
}

template<typename T, std::size_t I, std::size_t N>
struct member_accessor;

template<typename T, std::size_t I>
struct member_accessor<T, I, 0> {
    static decltype(auto) get(const T& obj) {
        ulx::log::expect(I >= 0, "autohasher.hpp: member_accessor<T, I, 0>: get(const T&): index out of bounds");
    }
};

template<typename T, std::size_t I, std::size_t N>
struct member_accessor {
    static decltype(auto) get(const T& obj) {
        if constexpr (member_count<T>() == N) {
            const auto& [a0, a1, a2, a3, a4, a5, a6, a7,
                         a8, a9, a10, a11, a12, a13, a14, a15,
                         a16, a17, a18, a19, a20, a21, a22, a23,
                         a24, a25, a26, a27, a28, a29, a30, a31,
                         a32, a33, a34, a35, a36, a37, a38, a39,
                         a40, a41, a42, a43, a44, a45, a46, a47,
                         a48, a49, a50, a51, a52, a53, a54, a55,
                         a56, a57, a58, a59, a60, a61, a62, a63] = obj;
            
            if constexpr (I == 0) return a0;
            else if constexpr (I == 1) return a1;
            else if constexpr (I == 2) return a2;
            else if constexpr (I == 3) return a3;
            else if constexpr (I == 4) return a4;
            else if constexpr (I == 5) return a5;
            else if constexpr (I == 6) return a6;
            else if constexpr (I == 7) return a7;
            else if constexpr (I == 8) return a8;
            else if constexpr (I == 9) return a9;
            else if constexpr (I == 10) return a10;
            else if constexpr (I == 11) return a11;
            else if constexpr (I == 12) return a12;
            else if constexpr (I == 13) return a13;
            else if constexpr (I == 14) return a14;
            else if constexpr (I == 15) return a15;
            else if constexpr (I == 16) return a16;
            else if constexpr (I == 17) return a17;
            else if constexpr (I == 18) return a18;
            else if constexpr (I == 19) return a19;
            else if constexpr (I == 20) return a20;
            else if constexpr (I == 21) return a21;
            else if constexpr (I == 22) return a22;
            else if constexpr (I == 23) return a23;
            else if constexpr (I == 24) return a24;
            else if constexpr (I == 25) return a25;
            else if constexpr (I == 26) return a26;
            else if constexpr (I == 27) return a27;
            else if constexpr (I == 28) return a28;
            else if constexpr (I == 29) return a29;
            else if constexpr (I == 30) return a30;
            else if constexpr (I == 31) return a31;
            else if constexpr (I == 32) return a32;
            else if constexpr (I == 33) return a33;
            else if constexpr (I == 34) return a34;
            else if constexpr (I == 35) return a35;
            else if constexpr (I == 36) return a36;
            else if constexpr (I == 37) return a37;
            else if constexpr (I == 38) return a38;
            else if constexpr (I == 39) return a39;
            else if constexpr (I == 40) return a40;
            else if constexpr (I == 41) return a41;
            else if constexpr (I == 42) return a42;
            else if constexpr (I == 43) return a43;
            else if constexpr (I == 44) return a44;
            else if constexpr (I == 45) return a45;
            else if constexpr (I == 46) return a46;
            else if constexpr (I == 47) return a47;
            else if constexpr (I == 48) return a48;
            else if constexpr (I == 49) return a49;
            else if constexpr (I == 50) return a50;
            else if constexpr (I == 51) return a51;
            else if constexpr (I == 52) return a52;
            else if constexpr (I == 53) return a53;
            else if constexpr (I == 54) return a54;
            else if constexpr (I == 55) return a55;
            else if constexpr (I == 56) return a56;
            else if constexpr (I == 57) return a57;
            else if constexpr (I == 58) return a58;
            else if constexpr (I == 59) return a59;
            else if constexpr (I == 60) return a60;
            else if constexpr (I == 61) return a61;
            else if constexpr (I == 62) return a62;
            else return a63;
        } else return member_accessor<T, I, N - 1>::get(obj);
    }
};

template<typename T, std::size_t I>
decltype(auto) get_member(const T& obj) {
    return member_accessor<T, I, 64>::get(obj);
}

template<typename T> 
    requires std::is_class_v<T> && (!std::is_fundamental_v<T>)
struct std::hash<T> {
    auto operator()(const T& obj) const noexcept -> ulx::u64 {
        constexpr auto N = member_count<T>();
        return [&]<ulx::u64... Is>(std::index_sequence<Is...>) {
            ulx::u64 seed = 0;
            ((hash_combine(seed, std::hash<std::decay_t<decltype(get_member<T, Is>(obj))>>{}(get_member<T, Is>(obj)))), ...);
            return seed;
        }(std::make_index_sequence<N>{});
    }
};