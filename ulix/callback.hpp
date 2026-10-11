#pragma once
#include "log.hpp"

namespace ulx {
    template<typename, u64 = 64> class callback;

    template<typename R, typename... As, u64 max_size>
    class callback<R(As...), max_size> {
    private:
        using invoker_fn = R(*)(void*, As...);
        using destroyer_fn = void(*)(void*);
        
        alignas(std::max_align_t) unsigned char buffer[max_size];
        invoker_fn invoker = nullptr;
        destroyer_fn destroyer = nullptr;

    public:
        inline constexpr callback() = default;

        template<typename F>
            requires (!std::is_same_v<std::decay_t<F>, callback>)
        inline constexpr callback(F&& f) {
            using decayed = std::decay_t<F>;
            log::expect(sizeof(decayed) <= max_size, "callback.hpp: callback(): decayed: callback too large for stack storage");
            
            new (&buffer) decayed(std::forward<F>(f));
            invoker = [](void* buf, As... args) -> R { return (*static_cast<decayed*>(buf))(std::forward<As>(args)...); };
            destroyer = [](void* buf) { static_cast<decayed*>(buf)->~decayed(); };
        }

        inline constexpr ~callback() {
            if (destroyer) destroyer(&buffer);
        }

        template<typename... Cs>
        inline constexpr auto operator()(Cs&&... args) const -> R {
            log::expect(invoker, "callback.hpp: operator(): fn: invalid callback");
            return invoker(const_cast<unsigned char*>(buffer), std::forward<Cs>(args)...);
        }

        inline constexpr auto valid() const -> bool {
            return invoker != nullptr;
        }
    };
}
