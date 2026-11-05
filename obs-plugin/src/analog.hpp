#pragma once
#include "input_mapping.hpp"
#include <algorithm>
#include <cstdint>

namespace keycast {
struct AnalogSample {
    uint32_t scan_code = 0;
    float depth = 0.0f;
};
inline void apply_analog(const AnalogSample &sample, const InputMapper &mapper,
                         AnimationState &state) {
    mapper.apply(InputKind::Analog, sample.scan_code,
                 std::clamp(sample.depth, 0.0f, 1.0f), state);
}
}
