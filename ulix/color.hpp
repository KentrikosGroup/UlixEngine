#pragma once
#include <glm/ext/vector_float4.hpp>
#include <ulix/types.hpp>
#include <ulix/autohasher.hpp>

namespace ulx {
    class color {
        f32 m_r, 
            m_g, 
            m_b, 
            m_a;
    
        public: inline constexpr color() = default;
        public: inline constexpr color(f32 r, f32 g, f32 b, f32 a = 255): m_r(r), m_g(g), m_b(b), m_a(a) {}
            
        public: inline constexpr auto r() const -> f32 { return m_r; }
        public: inline constexpr auto g() const -> f32 { return m_g; }
        public: inline constexpr auto b() const -> f32 { return m_b; }
        public: inline constexpr auto a() const -> f32 { return m_a; }

        public: inline constexpr auto r255() const -> f32 { return m_r; }
        public: inline constexpr auto g255() const -> f32 { return m_g; }
        public: inline constexpr auto b255() const -> f32 { return m_b; }
        public: inline constexpr auto a255() const -> f32 { return m_a; }
            
        public: inline constexpr auto r1() const -> f32 { return m_r / 255.0f; }
        public: inline constexpr auto g1() const -> f32 { return m_g / 255.0f; }
        public: inline constexpr auto b1() const -> f32 { return m_b / 255.0f; }
        public: inline constexpr auto a1() const -> f32 { return m_a / 255.0f; }
    
        public: inline constexpr static auto empty() -> color { return color(0, 0, 0, 0); }
        public: inline constexpr auto to_vec4() const -> glm::vec4 { return {r1(), g1(), b1(), a1()}; }
            
        public: inline constexpr auto operator==(const color& other) const -> bool = default;
    };
}

namespace std {
    template<>
    struct hash<ulx::color> {
        size_t operator()(const ulx::color& c) const noexcept {
            return ulx::make_hash(c.r(), c.g(), c.b(), c.a());
        }
    };
}
