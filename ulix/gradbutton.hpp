#pragma once
#include "ulix/requires.hpp"
#include <ulix/element.hpp>
#include <ulix/log.hpp>
#include <ulix/align.hpp>
#include <ulix/cursor.hpp>
#include <ulix/color.hpp>
#include <ulix/effents.hpp>
#include <ulix/context.hpp>

namespace ulx {
    class gradbutton {
        clicked clicked_state;
        color empty_color;
        gradcolor background;
        element::event::mouse clicked_event;
        color pressed_color;
        color hovered_color;

        public: template<typename _color>
            requires expect<_color, color>
        inline constexpr gradbutton(_color&& start): background(std::forward<_color>(start)), empty_color(std::forward<_color>(start)) {}
            
        public: template<typename _clicked, typename _pressed_color, typename _hovered_color>
            requires expect<_clicked, element::event::mouse> &&
                     expect<_pressed_color, color> &&
                     expect<_hovered_color, color>
        inline constexpr auto element(ulx::strvw id, _clicked&& clicked_event, _pressed_color&& pressed_color, _hovered_color&& hovered_color) -> ulx::element {
            this->clicked_event = std::forward<_clicked>(clicked_event);
            this->pressed_color = std::forward<_pressed_color>(pressed_color);
            this->hovered_color = std::forward<_hovered_color>(hovered_color);

            auto obj = ulx::element::unbordered(id)
                .call([this](ulx::element& slf) {
                    clicked_state.usefor(slf, this->clicked_event, 
                        event_pressed [this](ulx::context& ctx, ulx::element& slf) {
                            background.pressed(slf, ulx::color{22, 22, 22, 255});
                            slf.back(background.to(
                                this->pressed_color, 0));
                            slf.cur(ulx::handcur);
                        }, event_released [this](ulx::context& ctx, ulx::element& slf) {
                            slf.back(background.to(
                                empty_color, 150));
                            slf.cur(ulx::resetcur);
                            background.released(slf, {22, 22, 22, 255});
                        }, event_hovered [this](ulx::context& ctx, ulx::element& slf) {
                            background.hovered(slf, ulx::color{22, 22, 22, 255});
                            slf.back(background.to(
                                this->hovered_color, 150));
                            slf.cur(ulx::handcur);
                        }
                    );
                });
            return obj;
        }
    };
}
