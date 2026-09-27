#pragma once

#include "requires.hpp"
#include "scene.hpp"
#include "types.hpp"
#include "pixmap.hpp"
#include "callback.hpp"

namespace ulx {
    class context;
    
    class renderinfo {
        public:
            using render_callback_type = ulx::callback<ulx::scene(context&)>;
        
        private:
            render_callback_type render_callback;
            ulx::u32 max_texture_count = UINT16_MAX;

        public:
            renderinfo() = default;

            template<typename RC>
                requires ulx::expect<RC, render_callback_type>
            renderinfo(RC&& render_callback): render_callback(std::forward<RC>(render_callback)) {}

        public:
            inline auto max_textures(ulx::u32 count) -> renderinfo& {
                this->max_texture_count = count;
                return *this;
            }

        public:
            inline auto get_render_callback() const -> render_callback_type { return render_callback; }
            inline auto get_max_texture_count() const -> ulx::u32 { return max_texture_count; }
    };
}
