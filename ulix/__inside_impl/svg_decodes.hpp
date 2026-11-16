#pragma once
#include <ulix/path.hpp>
#include <ulix/pack.hpp>
#include <ulix/__inside_impl/log_utils.hpp>
#include <ulix/__required_libraries/resvg/resvg.hpp>

namespace ulx::sds {
    inline static const resvg_options* get_options() {
        static resvg_options* options = []{
            resvg_options* temp = resvg_options_create();
            log::uassert(temp != nullptr, "create svg options failed");
            resvg_options_load_system_fonts(temp);
            resvg_options_set_dpi(temp, 96.0f);
            return temp;
        }();
        return options;
    }
    
    inline static constexpr auto generate(const path& image, u32 width, u32 height) -> pack<vec<u8>, u32, u32> {
        pathstr file_path = image.str();
        resvg_render_tree* tree = nullptr;
        if (resvg_parse_tree_from_file(file_path.cstr(), get_options(), &tree) != RESVG_OK) {
            log::uerror("parse svg tree from file {} failed, returning as a 1x1 empty pixmap", image.str().cstr());
            return { vec<u8>(4, 0), 1, 1 };
        }

        resvg_size size = resvg_get_image_size(tree);
        u32 canvas_w = width * 2;
        u32 canvas_h = height * 2;
        f32 scale = std::min(f32(canvas_w) / size.width, f32(canvas_h) / size.height);

        f32 offset_x = (canvas_w - size.width * scale) * 0.5f;
        f32 offset_y = (canvas_h - size.height * scale) * 0.5f;

        resvg_transform transform = {};
        transform.a = scale;
        transform.b = 0.0f;
        transform.c = 0.0f;
        transform.d = scale;
        transform.e = offset_x;
        transform.f = offset_y;

        u32 stride = canvas_w * 4;
        vec<u8> pixels(stride * canvas_h, 0);
        resvg_render(tree, transform, canvas_w, canvas_h, reinterpret_cast<char*>(pixels.data()));
        resvg_tree_destroy(tree);

        return {std::move(pixels), canvas_w, canvas_h};
    }
}