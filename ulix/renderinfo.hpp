#pragma once

#include <types.hpp>
#include <pixmap.hpp>

namespace ulx {
    class renderinfo {
        private:
            void* render_callback;
            ulx::u32 max_texture_count = UINT16_MAX;
    
        public:
            renderinfo() = default;
    
            template<typename RenderCallback>
            renderinfo(RenderCallback render_callback): render_callback(reinterpret_cast<void*>(render_callback)) {}
    
        public:
            inline auto max_textures(ulx::u32 count) -> renderinfo& {
                this->max_texture_count = count;
                return *this;
            }
    
        public:
            inline auto get_render_callback() const -> void* { return render_callback; }
            inline auto get_max_texture_count() const -> ulx::u32 { return max_texture_count; }
    };
}