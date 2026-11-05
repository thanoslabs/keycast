#pragma once
#include "input_events.hpp"
#include <algorithm>
namespace keycast {
// Analog input takes priority only when fresh/available; digital remains
// usable for unsupported keyboards and Wooting SDK disconnects.
inline float effective_key_depth(const InputState &inputs, uint32_t code,
                                 bool analog_available) {
    const float digital = inputs.amount(InputKind::Key, code);
    return analog_available
        ? std::clamp(inputs.amount(InputKind::Analog, code), 0.0f, 1.0f)
        : digital;
}
}
