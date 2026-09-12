#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in float in_corner_radius;
layout(location = 3) in vec2 in_half_size;
layout(location = 4) in float in_border_width;
layout(location = 5) in vec4 in_border_color;
layout(location = 6) in vec2 in_center;
layout(location = 7) in vec2 in_uv;
layout(location = 8) in uint in_tex_index;

layout(location = 0) out vec2 fragment_position;
layout(location = 1) out vec4 fragment_color;
layout(location = 2) out float fragment_corner_radius;
layout(location = 3) out vec2 fragment_half_size;
layout(location = 4) out float fragment_border_width;
layout(location = 5) out vec4 fragment_border_color;
layout(location = 6) out vec2 fragment_center;
layout(location = 7) out vec2 fragment_uv;
layout(location = 8) out uint fragment_tex_index;


layout(push_constant) uniform PushConstant {
    mat4 projection;
} push_constant;

void main() {
    gl_Position = push_constant.projection * vec4(in_position, 0.0, 1.0);

    fragment_position = in_position;
    fragment_color = in_color;
    fragment_corner_radius = in_corner_radius;
    fragment_half_size = in_half_size;
    fragment_border_width = in_border_width;
    fragment_border_color = in_border_color;
    fragment_uv = in_uv;
    fragment_tex_index = in_tex_index;
    fragment_center = in_center;
}
