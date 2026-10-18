#pragma once
#include "requires.hpp"
#include "scene.hpp"
#include "types.hpp"
#include "callback.hpp"
#include "present_mode.hpp"

namespace ulx {
    class context;
    class renderinfo {
        public: using render_callback = ulx::callback<ulx::scene(context&)>;
        
        private:
        render_callback renderer;
        ulx::u32 max_texture_count = UINT16_MAX;
        ulx::present_mode pmode = ulx::present_mode::prefer_mailmode;

        public: inline constexpr renderinfo() = default;
        public: template<typename _callback>
            requires ulx::expect<_callback, render_callback>
        inline constexpr renderinfo(_callback&& render_callback){
            renderer = std::forward<_callback>(render_callback);
        }

        public: inline constexpr auto textures(ulx::u32 max) -> renderinfo& {
            this->max_texture_count = max;
            return *this;
        }

        public: inline constexpr auto present(ulx::present_mode mode) -> renderinfo& {
            this->pmode = mode;
            return *this;
        }

        public: inline constexpr auto get_render_callback() const -> const render_callback& { return renderer; }
        public: inline constexpr auto get_max_texture_count() const -> ulx::u32 { return max_texture_count; }
        public: inline constexpr auto get_present_mode() const -> ulx::present_mode { return pmode; }
    };
}
