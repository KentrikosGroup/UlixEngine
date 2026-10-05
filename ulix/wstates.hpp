#pragma once
#include "callback.hpp"
#include "color.hpp"
#include "easing.hpp"
#include "object.hpp"
#include "requires.hpp"

#define event_pressed
#define event_released
#define event_hovered
#define event_clicked

namespace ulx::wst {
    class gradcolor {
        public:
            using easing_callback = ulx::callback<ulx::f32(ulx::f32)>;

        private:
            ulx::color current_color;
            ulx::color start_color;
            ulx::color target_color;
            ulx::u64 start_time = 0;
            ulx::u64 duration = 1000;
            bool animating = false;

        public:
            inline constexpr gradcolor() = default;

            template<typename C>
                requires ulx::expect<C, ulx::color>
            inline constexpr gradcolor(C&& start): 
                current_color(std::forward<C>(start)),
                start_color(std::forward<C>(start)) {}
                
            inline constexpr auto to(ulx::color target, ulx::u64 duration, const easing_callback& easing = ulx::eas::inout2) -> ulx::color {
                if (!animating || target != target_color) {
                    start_color = current_color;
                    target_color = target;
                    duration = duration;
                    start_time = get_time();
                    animating = true;
                }

                ulx::u64 elapsed = get_time() - start_time;
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

            inline constexpr auto pressed(ulx::object& object, const ulx::color& empty) -> void {
                if (current_color == ulx::color::empty())
                    object.back(empty);
            }

            inline constexpr auto released(ulx::object& object, const ulx::color& empty) -> void {
                if (current_color == empty)
                    object.back(ulx::color::empty());
            }

            inline constexpr auto hovered(ulx::object& object, const ulx::color& empty) -> void {
                if (current_color == ulx::color::empty())
                    object.back(empty);
            }

            inline constexpr auto color() -> ulx::color& {
                return current_color;
            }

        private:
            inline constexpr ulx::u64 get_time() {
                auto now = std::chrono::steady_clock::now();
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count();
            }
    };

    class clicked {
        private:
            bool is_pressed = false;
            ulx::object::pressed_event pressed_e;
            ulx::object::released_event released_e;
            ulx::object::hovered_event hovered_e;
            ulx::object::mouse_event clicked_e;

        public:
            template<typename P, typename R, typename H, typename C>
                requires ulx::expect<P, ulx::object::pressed_event> &&
                         ulx::expect<R, ulx::object::released_event> &&
                         ulx::expect<H, ulx::object::hovered_event> &&
                         ulx::expect<C, ulx::object::mouse_event>
            inline constexpr auto usefor(ulx::object& object, C&& clicked_event, P&& pressed_event, R&& released_event, H&& hovered_event) -> void {
                pressed_e = std::forward<P>(pressed_event);
                released_e = std::forward<R>(released_event);
                hovered_e = std::forward<H>(hovered_event);
                clicked_e = std::forward<C>(clicked_event);
                
                object.pressed([this](ulx::context& ctx, ulx::object& slf) {
                    pressed();
                    pressed_e(ctx, slf);
                }).released([this](ulx::context& ctx, ulx::object& slf) {
                    released();
                    released_e(ctx, slf);
                }).hovered([this](ulx::context& ctx, ulx::object& slf) {
                    hovered_e(ctx, slf);
                    if (hovered()) clicked_e(ctx, slf);
                });
            }

            template<typename C>
                requires ulx::expect<C, ulx::object::hovered_event>
            inline constexpr auto usefor(ulx::object& object, C&& clicked_event) -> void {
                usefor(object, clicked_event,
                    ulx::object::pressed_event(),
                    ulx::object::released_event(),
                    ulx::object::hovered_event());
            }

        public:
            inline constexpr auto pressed() -> void {
                is_pressed = true;
            }

            inline constexpr auto released() -> void {
                is_pressed = false;
            }

            inline constexpr auto hovered() -> bool {
                if (is_pressed == true) {
                    is_pressed = false;
                    return true;
                }
                return false;
            }
    };
}
