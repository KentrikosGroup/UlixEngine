#pragma once
#include "log.hpp"
#include "requires.hpp"

namespace ulx {
    template<typename, ulx::size = 64> class callback;

    template<typename Ret, typename... Args, ulx::size max_size>
    class callback<Ret(Args...), max_size> {
    private:
        using invoker_fn = Ret(*)(void*, Args...);
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
            ulx::log::expect(sizeof(decayed) <= max_size, "callback.hpp: callback(): decayed: callback too large for stack storage");
            
            new (&buffer) decayed(std::forward<F>(f));
            invoker = [](void* buf, Args... args) -> Ret { return (*static_cast<decayed*>(buf))(std::forward<Args>(args)...); };
            destroyer = [](void* buf) { static_cast<decayed*>(buf)->~decayed(); };
        }

        inline constexpr ~callback() {
            if (destroyer) destroyer(&buffer);
        }

        template<typename... CallArgs>
        inline constexpr auto operator()(CallArgs&&... args) const -> Ret {
            ulx::log::expect(invoker, "callback.hpp: operator(): fn: invalid callback");
            return invoker(const_cast<unsigned char*>(buffer), std::forward<CallArgs>(args)...);
        }

        inline constexpr auto valid() const -> bool {
            return invoker != nullptr;
        }
    };
}

// namespace ulx {
//     template<typename F> class callback {
//         private:
//             std::decay_t<F> fn = nullptr;

//         public:
//             inline constexpr callback() = default;

//             template<typename U>
//                 requires ulx::expect<U, std::decay_t<F>>
//             inline constexpr callback(U&& f) : fn(std::forward<U>(f)) {}

//             template<typename... Args>
//             inline constexpr auto operator()(Args&&... args) const -> decltype(auto) {
//                 ulx::log::expect(fn, "callback.hpp: operator(): fn: invalid callback");
//                 return fn(std::forward<Args>(args)...);
//             }

//             inline constexpr auto valid() const -> bool {
//                 return fn != nullptr;
//             }
//     };
// }
