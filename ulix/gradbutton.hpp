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
        strvw                 m_id;
        clicked               m_clicked;
        color                 m_empty;
        gradcolor             m_grad_back;
        element::event::mouse m_clicked_event;
        color                 m_pressed_color;
        color                 m_hovered_color;

        public: template<typename t_start_color>
            requires expect<t_start_color, color>
        inline constexpr gradbutton(strvw id, t_start_color&& start): 
            m_id(id),
            m_grad_back(std::forward<t_start_color>(start)), 
            m_empty(std::forward<t_start_color>(start)) {}

        public: template<typename t_clicked_event, typename _pressed_color, typename _hovered_color>
            requires expect<t_clicked_event, element::event::mouse> &&
                     expect<_pressed_color, color> &&
                     expect<_hovered_color, color>
        inline constexpr auto create(t_clicked_event&& clicked_event, _pressed_color&& pressed_color, _hovered_color&& hovered_color) -> ulx::element {
            this->m_clicked_event = std::forward<t_clicked_event>(clicked_event);
            this->m_pressed_color = std::forward<_pressed_color>(pressed_color);
            this->m_hovered_color = std::forward<_hovered_color>(hovered_color);

            auto obj = ulx::element(m_id)
                .call([this](ulx::element& slf) {
                    m_clicked.usefor(slf, this->m_clicked_event,
                        [this] ulx_lambda_pressed {
                            m_grad_back.pressed(slf, ulx::color{22, 22, 22, 255});
                            slf.back(m_grad_back.to(
                                this->m_pressed_color, 0));
                            slf.cur(ulx::handcur);
                        },
                        [this] ulx_lambda_released {
                            slf.back(m_grad_back.to(
                                m_empty, 150));
                            slf.cur(ulx::resetcur);
                            m_grad_back.released(slf, {22, 22, 22, 255});
                        },
                        [this] ulx_lambda_hovered {
                            m_grad_back.hovered(slf, ulx::color{22, 22, 22, 255});
                            slf.back(m_grad_back.to(
                                this->m_hovered_color, 150));
                            slf.cur(ulx::handcur);
                        }
                    );
                });
            return obj;
        }
    };
}
