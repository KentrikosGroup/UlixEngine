#pragma once
#include <cmath>

namespace ulx {
    inline static constexpr float in2(float t) {
        return t * t;
    }

    inline static constexpr float in3(float t) {
        return t * t * t;
    }

    inline static constexpr float in4(float t) {
        return t * t * t * t;
    }

    inline static constexpr float out2(float t) {
        return 1 - (1 - t) * (1 - t);
    }

    inline static constexpr float out3(float t) {
        return 1 - (1 - t) * (1 - t) * (1 - t);
    }

    inline static constexpr float out4(float t) {
        return 1 - (1 - t) * (1 - t) * (1 - t) * (1 - t);
    }

    inline static constexpr float inout2(float t) {
        return t < 0.5 ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;
    }

    inline static constexpr float inout3(float t) {
        return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
    }

    inline static constexpr float inout4(float t) {
        return t < 0.5 ? 8 * t * t * t * t : 1 - pow(-2 * t + 2, 4) / 2;
    }

    inline static constexpr float outin2(float t) {
        return t < 0.5 ? 0.5f * (1 - std::pow(1 - 2 * t, 2)) : 0.5f * std::pow(2 * t - 1, 2) + 0.5f;
    }

    inline static constexpr float outin3(float t) {
        return t < 0.5 ? 0.5f * (1 - std::pow(1 - 2 * t, 3)) : 0.5f * std::pow(2 * t - 1, 3) + 0.5f;
    }

    inline static constexpr float outin4(float t) {
        return t < 0.5 ? 0.5f * (1 - std::pow(1 - 2 * t, 4)) : 0.5f * std::pow(2 * t - 1, 4) + 0.5f;
    }
}