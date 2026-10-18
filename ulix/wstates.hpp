#pragma once
#include "callback.hpp"
#include "color.hpp"
#include "easing.hpp"
#include "element.hpp"
#include "types.hpp"
#include <utility>

#define event_pressed
#define event_released
#define event_hovered
#define event_clicked

namespace ulx::wst {
    class gradcolor {
        color current_color;
        color start_color;
        color target_color;
        u64 start_time = 0;
        u64 duration = 1000;
        bool animating = false;

        public: using easing_callback = callback<f32(f32)>;

        public: inline constexpr gradcolor() = default;
        public: template<typename _color>
            requires ulx::expect<_color, color>
        inline constexpr gradcolor(_color&& start) {
            current_color = std::forward<_color>(start);
            start_color = std::forward<_color>(start);
        }
                
        public: template<typename _color>
            requires ulx::expect<_color, color>
        inline constexpr auto to(_color&& target, u64 duration, const easing_callback& easing = eas::inout2) -> color {
            if (!animating || target != target_color) {
                start_color = current_color;
                target_color = target;
                duration = duration;
                start_time = get_time();
                animating = true;
            }

            u64 elapsed = get_time() - start_time;
            if (elapsed >= duration) {
                current_color = target_color;
                animating = false;
                return current_color;
            }

            float delta = static_cast<float>(elapsed) / static_cast<float>(duration);
            delta = easing(delta);
            current_color = {
                start_color.red() + (target_color.red() - start_color.red()) * delta,
                start_color.green() + (target_color.green() - start_color.green()) * delta,
                start_color.blue() + (target_color.blue() - start_color.blue()) * delta,
                start_color.alpha() + (target_color.alpha() - start_color.alpha()) * delta
            };

            return current_color;
        }

        public: template<typename _color>
            requires ulx::expect<_color, color>
        inline constexpr auto pressed(element& object, _color&& empty) -> void {
            if (current_color == color::empty())
                object.back(std::forward<_color>(empty));
        }

        public: inline constexpr auto released(element& object, const color& empty) -> void {
            if (current_color == empty)
                object.back(color::empty());
        }

        public: template<typename _color>
            requires ulx::expect<_color, color>
        inline constexpr auto hovered(element& object, _color&& empty) -> void {
            if (current_color == color::empty())
                object.back(std::forward<_color>(empty));
        }

        public: inline constexpr auto color() -> color& {
            return current_color;
        }

        private: inline constexpr u64 get_time() {
            return ms_cast(steady::now().time_since_epoch()).count();
        }
    };

    class clicked {
        bool is_pressed = false;
        element::event::pressed pressed_e;
        element::event::released released_e;
        element::event::hovered hovered_e;
        element::event::mouse clicked_e;

        public: template<typename _event1, typename _event2, typename _event3, typename _event4>
            requires ulx::expect<_event1, element::event::mouse> &&
                     ulx::expect<_event2, element::event::pressed> &&
                     ulx::expect<_event3, element::event::released> &&
                     ulx::expect<_event4, element::event::hovered>
        inline constexpr auto usefor(element& object, _event1&& clicked_event, 
            _event2&& pressed_event = element::event::pressed(), 
            _event3&& released_event = element::event::released(), 
            _event4&& hovered_event = element::event::hovered()
        ) -> void {
            clicked_e = std::forward<_event1>(clicked_event);
            pressed_e = std::forward<_event2>(pressed_event);
            released_e = std::forward<_event3>(released_event);
            hovered_e = std::forward<_event4>(hovered_event);
            
            object.pressed([this](context& ctx, element& slf) {
                pressed();
                pressed_e(ctx, slf);
            }).released([this](context& ctx, element& slf) {
                released();
                released_e(ctx, slf);
            }).hovered([this](context& ctx, element& slf) {
                hovered_e(ctx, slf);
                if (hovered()) clicked_e(ctx, slf);
            });
        }

        public: inline constexpr auto pressed() -> void {
            is_pressed = true;
        }

        public: inline constexpr auto released() -> void {
            is_pressed = false;
        }

        public: inline constexpr auto hovered() -> bool {
            if (is_pressed == true) {
                is_pressed = false;
                return true;
            }
            return false;
        }
    };
}
