#pragma once

#include "types.hpp"
#include <chrono>

namespace chrono = std::chrono;

namespace ulx {
    template<typename... Args> class timer {
        private:
            using TimeoutTask = void(*)(timer&, Args&...);
    
        private:
            ulx::i64 last_time;
            ulx::u64 last_timeout_delay = 0;
            ulx::u64 timeout_delay = 0;
            ulx::vec<TimeoutTask> timeout_events;
    
        public:
            inline constexpr timer():
                last_time(chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count())
            {}
    
            inline auto get_delta() -> ulx::i64 {
                ulx::i64 current_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
                return current_time - last_time;
            }
    
            inline auto rmv_events() -> void {
                timeout_events = {};
            }
    
            inline auto pop_event() -> void {
                timeout_events.pop_back();
            }
    
            inline auto delay(ulx::u64 delay) -> timer& {
                timeout_delay = delay;
                return *this;
            }
    
            inline auto event(TimeoutTask event) -> timer& {
                timeout_events.push_back(event);
                return *this;
            }
    
        public:
            inline void update(Args&... args) {
                if (get_delta() > static_cast<ulx::i32>(last_timeout_delay) && !timeout_events.empty()) {
                    last_timeout_delay += timeout_delay;
                    for (const auto& event : timeout_events)
                        event(*this, args...);
                }
            }
    };
}
