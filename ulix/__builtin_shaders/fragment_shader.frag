#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout(set = 0, binding = 0) uniform sampler2D u_textures[];
layout(location = 0)         in vec2      fragment_position;
layout(location = 1)         in vec4      fragment_color;
layout(location = 2)         in float     fragment_corner_radius;
layout(location = 3)         in vec2      fragment_half_size;
layout(location = 4)         in float     fragment_border_width;
layout(location = 5)         in vec4      fragment_border_color;
layout(location = 6)         in vec2      fragment_center;
layout(location = 7)         in vec2      fragment_uv;
layout(location = 8)         flat in uint fragment_tex_index;
layout(location = 9)         flat in uint fragment_elemtype;
layout(location = 10)        in vec4      fragment_data;
layout(location = 0) out vec4 out_color;

#define charelem 0
#define svgelem 1
#define pixmelem 2
#define shadelem 3

float rounded_box_sdf(vec2 position, vec2 bounds, float radius) {
    vec2 delta = abs(position) - bounds + radius;
    return length(max(delta, 0.0)) + min(max(delta.x, delta.y), 0.0) - radius;
}

void normal_element() {
    vec2 local_position = fragment_position - fragment_center;
    float dist = rounded_box_sdf(local_position, fragment_half_size, fragment_corner_radius);
    float fw = fwidth(dist);

    float inner = dist + fragment_border_width;
    float fill_alpha = 1.0 - smoothstep(-fw, fw, dist);
    float border_alpha = smoothstep(-fw, fw, inner) - smoothstep(-fw, fw, dist);

    vec4 tex_color = texture(u_textures[fragment_tex_index], fragment_uv);
    switch (fragment_elemtype) {
        case charelem: {
            vec2 tex_size = textureSize(u_textures[fragment_tex_index], 0);
            vec2 inv_tex_size = 1.0 / tex_size;
            
            float t0 = texture(u_textures[fragment_tex_index], fragment_uv - vec2(inv_tex_size.x * 2.0, 0.0)).r;
            float t1 = texture(u_textures[fragment_tex_index], fragment_uv - vec2(inv_tex_size.x, 0.0)).r;
            float t2 = tex_color.r;
            float t3 = texture(u_textures[fragment_tex_index], fragment_uv + vec2(inv_tex_size.x, 0.0)).r;
            float t4 = texture(u_textures[fragment_tex_index], fragment_uv + vec2(inv_tex_size.x * 2.0, 0.0)).r;
            
            const float w0 = 0.274;
            const float w1 = 0.452;
            const float w2 = 0.274;
            
            float c  = w0 * t1 + w1 * t2 + w2 * t3;
            float cl = w0 * t0 + w1 * t1 + w2 * t2;
            float cr = w0 * t2 + w1 * t3 + w2 * t4;
            
            float edge = (cr - cl) * 0.35;
            float edge_mask = c * (1.0 - c) * 4.0;
            edge_mask = clamp(edge_mask * edge_mask, 0.0, 1.0);

            float strength = 1.8;
            float r = clamp(c - edge * edge_mask * strength, 0.0, 1.0);
            float g = c;
            float b = clamp(c + edge * edge_mask * strength, 0.0, 1.0);
            
            vec3 subpixel = vec3(r, g, b);
            const vec3 LUMA = vec3(0.229, 0.687, 0.084);
            float luma = dot(subpixel, LUMA);
            subpixel = mix(subpixel, vec3(luma), 0.12);
            
            subpixel = pow(subpixel, vec3(1.0 / 1.5));
            subpixel = clamp((subpixel - 0.49) * 1.21 + 0.49, 0.0, 1.0);
            
            float alpha = max(max(subpixel.r, subpixel.g), subpixel.b);
            tex_color = vec4(subpixel * fragment_data.rgb, alpha * fragment_data.a);
            break;
        } case svgelem: {
            tex_color.rgb /= max(tex_color.a, 1e-7);
            break;
        } case pixmelem: {
            float sigma = fragment_data.x;
            int radius = int(fragment_data.y);
            int taps = int(fragment_data.z);
            
            float total_weight = 0.0;
            float weights[64];
            for (int i = 0; i < taps; i++) {
                int offset = i - radius;
                float x = float(offset);
                float w = exp(-(x * x) / (2.0 * sigma * sigma));
                weights[i] = w;
                total_weight += w;
            }
            for (int i = 0; i < taps; i++) {
                weights[i] /= total_weight;
            }
            
            vec2 tex_size = textureSize(u_textures[fragment_tex_index], 0);
            vec2 inv = 1.0 / tex_size;
            
            vec4 horizontal = vec4(0.0);
            for (int i = 0; i < taps; i++) {
                int offset = i - radius;
                vec2 uv = fragment_uv + vec2(float(offset) * inv.x, 0.0);
                horizontal += weights[i] * texture(u_textures[fragment_tex_index], uv);
            }
            
            vec4 result = vec4(0.0);
            for (int i = 0; i < taps; i++) {
                int offset = i - radius;
                vec2 uv = fragment_uv + vec2(float(offset) * inv.x, 0.0);
                vec4 vertical_pass = vec4(0.0);
                for (int j = 0; j < taps; j++) {
                    int voffset = j - radius;
                    vec2 vuv = uv + vec2(0.0, float(voffset) * inv.y);
                    vertical_pass += weights[j] * texture(u_textures[fragment_tex_index], vuv);
                }
                result += weights[i] * vertical_pass;
            }
            
            tex_color = result;
            break;
        } default: break;
    }

    vec3 mixed_rgb = mix(fragment_color.rgb, tex_color.rgb, tex_color.a);
    float mixed_alpha = mix(fragment_color.a, 1.0, tex_color.a);

    float fill_out_alpha = mixed_alpha * fill_alpha;
    vec3 fill_rgb = mixed_rgb * fill_out_alpha;
    vec3 border_rgb = fragment_border_color.rgb * (fragment_border_color.a * border_alpha);

    vec3 color = fill_rgb + border_rgb;
    float total_alpha = clamp(fill_out_alpha + border_alpha, 0.0, 1.0);
    out_color = vec4(color, total_alpha);
}

void shadow_element() {
    bool inset = fragment_data.x > 0.5;
    float blur_radius = fragment_border_width;
    float corner_radius = fragment_corner_radius;
    
    vec2 local_pos = fragment_position - fragment_center;
    float element_dist = rounded_box_sdf(local_pos, fragment_half_size, corner_radius);
    float fw = fwidth(element_dist);
    
    float shadow_alpha;
    if (inset) {
        shadow_alpha = smoothstep(-fw, fw, -element_dist);
        if (blur_radius > 0.0) {
            float fade = element_dist / max(blur_radius, 1e-5);
            shadow_alpha *= exp(-fade * fade * fragment_data.y);
        }
    } else {
        shadow_alpha = 1.0 - smoothstep(-fw, fw, element_dist);
        if (blur_radius > 0.0) {
            float fade = -element_dist / max(blur_radius, 1e-5);
            shadow_alpha *= 1.0 - exp(-fade * fade * fragment_data.y);
        }
    }
    
    vec3 shadow_color = fragment_color.rgb;
    float final_alpha = shadow_alpha * fragment_color.a;
    out_color = vec4(shadow_color * final_alpha, final_alpha);
}

void main() {
    if (fragment_elemtype == shadelem) {
        shadow_element();
        return;
    }
    normal_element();
}
