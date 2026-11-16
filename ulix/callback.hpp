#pragma once
#include <ulix/__inside_impl/log_utils.hpp>

namespace ulx {
    template<typename, u64 = 64> class callback;

    template<typename t_result, typename... t_args, u64 max_size>
    class callback<t_result(t_args...), max_size> {
        using invoker_fn = t_result(*)(void*, t_args...);
        using destroyer_fn = void(*)(void*);
        using copier_fn = void(*)(void*, const void*);
        
        alignas(std::max_align_t) mutable unsigned char buffer[max_size];
        invoker_fn invoker = nullptr;
        destroyer_fn destroyer = nullptr;
        copier_fn copier = nullptr;

        public: inline constexpr callback() = default;

        public: template<typename t_fn>
            requires (!std::is_same_v<std::decay_t<t_fn>, callback>)
        inline constexpr callback(t_fn&& f) {
            using decayed = std::decay_t<t_fn>;
            static_assert(sizeof(decayed) <= max_size, 
                "the size of the function is greater than maximum size");
            
            new (&buffer) decayed(std::forward<t_fn>(f));
            invoker = [](void* buf, t_args... args) -> t_result { return (*static_cast<decayed*>(buf))(std::forward<t_args>(args)...); };
            destroyer = [](void* buf) { static_cast<decayed*>(buf)->~decayed(); };
            copier = [](void* dst, const void* src) { new (dst) decayed(*static_cast<const decayed*>(src)); };
        }

        public: inline constexpr callback(const callback& other) {
            if (other.invoker) {
                other.copier(buffer, other.buffer);
                invoker = other.invoker;
                destroyer = other.destroyer;
                copier = other.copier;
            }
        }

        public: inline constexpr callback& operator=(const callback& other) {
            if (this != &other) {
                if (destroyer) destroyer(buffer);
                if (other.invoker) {
                    other.copier(buffer, other.buffer);
                    invoker = other.invoker;
                    destroyer = other.destroyer;
                    copier = other.copier;
                } else {
                    invoker = nullptr;
                    destroyer = nullptr;
                    copier = nullptr;
                }
            }
            return *this;
        }

        public: inline constexpr callback(callback&& other) noexcept {
            if (other.invoker) {
                std::memcpy(buffer, other.buffer, max_size);
                invoker = other.invoker;
                destroyer = other.destroyer;
                copier = other.copier;
                other.invoker = nullptr;
                other.destroyer = nullptr;
                other.copier = nullptr;
            }
        }
        
        public: inline constexpr callback& operator=(callback&& other) noexcept {
            if (this != &other) {
                if (destroyer) destroyer(buffer);
                if (other.invoker) {
                    std::memcpy(buffer, other.buffer, max_size);
                    invoker = other.invoker;
                    destroyer = other.destroyer;
                    copier = other.copier;
                    other.invoker = nullptr;
                    other.destroyer = nullptr;
                    other.copier = nullptr;
                } else {
                    invoker = nullptr;
                    destroyer = nullptr;
                    copier = nullptr;
                }
            }
            return *this;
        }

        public: inline constexpr ~callback() {
            if (destroyer) destroyer(&buffer);
        }

        public: template<typename... t_callargs>
        inline constexpr auto operator()(t_callargs&&... args) const -> t_result {
            if (invoker == nullptr) {
                log::uwarn("uninitialized callback, the program will execute normally");
                
                if constexpr (!std::is_void_v<t_result>) return {};
                else return;
            }
            return invoker(buffer, std::forward<t_callargs>(args)...);
        }

        public: inline constexpr auto valid() const -> bool {
            return invoker != nullptr;
        }
    };
}