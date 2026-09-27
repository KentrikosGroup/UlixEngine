#pragma once
#include "log.hpp"
#include "requires.hpp"

namespace ulx {
    template<typename F> class callback {
        private:
            std::decay_t<F> fn = nullptr;

        public:
            inline constexpr callback() = default;

            template<typename U>
                requires ulx::expect<U, std::decay_t<F>>
            inline constexpr callback(U&& f) : fn(std::forward<U>(f)) {}

            template<typename... Args>
            inline constexpr auto operator()(Args&&... args) -> decltype(auto) {
                ulx::log::expect(fn, "invalid callback");
                return fn(std::forward<Args>(args)...);
            }
    };
}
