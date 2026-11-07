#pragma once
#include "requires.hpp"
#include "scene.hpp"
#include "types.hpp"
#include "callback.hpp"

namespace ulx {
    class context;
    class renderinfo {
        public: using render_callback = ulx::callback<ulx::scene(context&)>;
        private: render_callback renderer;

        public: inline constexpr renderinfo() = default;
        public: template<typename _callback>
            requires ulx::expect<_callback, render_callback>
        inline constexpr renderinfo(_callback&& render_callback){
            renderer = std::forward<_callback>(render_callback);
        }

        public: inline constexpr auto get_render_callback() const -> const render_callback& { return renderer; }
    };
}
