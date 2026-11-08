#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout(set = 0, binding = 0) uniform sampler2D u_textures[];
layout(location = 0) in vec2 fragment_position;
layout(location = 1) in vec4 fragment_color;
layout(location = 2) in float fragment_corner_radius;
layout(location = 3) in vec2 fragment_half_size;
layout(location = 4) in float fragment_border_width;
layout(location = 5) in vec4 fragment_border_color;
layout(location = 6) in vec2 fragment_center;
layout(location = 7) in vec2 fragment_uv;
layout(location = 8) flat in uint fragment_tex_index;
layout(location = 9) flat in uint fragment_elemtype;
layout(location = 10) in vec4 fragment_data;

layout(location = 0) out vec4 out_color;

float rounded_box_sdf(vec2 position, vec2 bounds, float radius) {
    vec2 delta = abs(position) - bounds + radius;
    return length(max(delta, 0.0)) + min(max(delta.x, delta.y), 0.0) - radius;
}

#define normalelem 0
#define charelem 1
#define svgelem 2
#define pixmelem 3

void main() {
    vec2 local_position = fragment_position - fragment_center;
    float dist = rounded_box_sdf(local_position, fragment_half_size, fragment_corner_radius);
    float fw = fwidth(dist);

    float inner = dist + fragment_border_width;
    float fill_alpha = 1.0 - smoothstep(-fw, fw, dist);
    float border_alpha = smoothstep(-fw, fw, inner) - smoothstep(-fw, fw, dist);

    vec4 tex_color = texture(u_textures[fragment_tex_index], fragment_uv);
    if (fragment_elemtype == charelem) {
        vec2 tex_size = textureSize(u_textures[fragment_tex_index], 0);
        vec2 inv_tex_size = 1.0 / tex_size;
        
        float sigma = 0.425;
        float w0 = 0.274;
        float w1 = 0.452;
        float w2 = 0.274;
        
        float c = w0 * texture(u_textures[fragment_tex_index], fragment_uv - vec2(inv_tex_size.x, 0.0)).r
                + w1 * texture(u_textures[fragment_tex_index], fragment_uv).r
                + w2 * texture(u_textures[fragment_tex_index], fragment_uv + vec2(inv_tex_size.x, 0.0)).r;
        
        float cl = w0 * texture(u_textures[fragment_tex_index], fragment_uv - vec2(inv_tex_size.x * 2.0, 0.0)).r
                 + w1 * texture(u_textures[fragment_tex_index], fragment_uv - vec2(inv_tex_size.x, 0.0)).r
                 + w2 * texture(u_textures[fragment_tex_index], fragment_uv).r;
        
        float cr = w0 * texture(u_textures[fragment_tex_index], fragment_uv).r
                 + w1 * texture(u_textures[fragment_tex_index], fragment_uv + vec2(inv_tex_size.x, 0.0)).r
                 + w2 * texture(u_textures[fragment_tex_index], fragment_uv + vec2(inv_tex_size.x * 2.0, 0.0)).r;
        
        float edge = (cr - cl) * 0.35;
        float edge_mask = c * (1.0 - c) * 4.0;
        edge_mask = clamp(edge_mask * edge_mask, 0.0, 1.0);
        
        float r = clamp(c + edge * edge_mask * 1.5, 0.0, 1.0);
        float g = c;
        float b = clamp(c - edge * edge_mask * 1.5, 0.0, 1.0);
        
        vec3 subpixel = vec3(r, g, b);
        
        const vec3 LUMA = vec3(0.229, 0.687, 0.084);
        float luma = dot(subpixel, LUMA);
        subpixel = mix(subpixel, vec3(luma), 0.25);
        
        subpixel = pow(subpixel, vec3(1.0 / 1.5));
        subpixel = clamp((subpixel - 0.49) * 1.21 + 0.49, 0.0, 1.0);
        
        float alpha = max(max(subpixel.r, subpixel.g), subpixel.b);
        tex_color = vec4(subpixel * fragment_data.rgb, alpha * fragment_data.a);
        tex_color *= fragment_data;
    } else if (fragment_elemtype == svgelem) {
        if (tex_color.a > 0.0) tex_color.rgb /= tex_color.a;
        else tex_color.rgb = vec3(0.0);
    } else if (fragment_elemtype == pixmelem) {
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
