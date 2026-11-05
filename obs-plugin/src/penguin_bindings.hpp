#pragma once
#include "input_events.hpp"
#include "animation.hpp"
#include <algorithm>
namespace keycast {
// MVP default: W/A/S/D on left flipper, primary mouse on right flipper.
// Inputs are combined rather than last-event-wins; releasing W while A is
// held must not release the left flipper.
inline void update_penguin_targets(const InputState &inputs, AnimationState &animation) {
    float left = 0.0f;
    for (uint32_t code : {0x57u, 0x41u, 0x53u, 0x44u})
        left = std::max(left, inputs.amount(InputKind::Key, code));
    const float right = inputs.amount(InputKind::MouseButton, 1);
    animation.set_target("ParamFlipperLeft", left);
    animation.set_target("ParamFlipperRight", right);
    animation.set_target("ParamMouseClick", right);
    animation.set_target("ParamBodyBounce", std::max(left, right) * 0.35f);
}
}
