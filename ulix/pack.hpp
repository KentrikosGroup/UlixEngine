#pragma once
#include <tuple>

namespace ulx {
    template<typename... Ts>
    struct pack {
        std::tuple<Ts...> data;
    
        inline constexpr pack() = default;
        inline constexpr pack(Ts... args) : data(std::move(args)...) {}

        template<std::size_t I>
        inline constexpr decltype(auto) get() {
            return std::get<I>(data);
        }
        
        template<std::size_t I>
        inline constexpr decltype(auto) get() const {
            return std::get<I>(data);
        }
    
        inline constexpr auto operator==(const pack& other) const -> bool {
            return data == other.data;
        }
    };
}