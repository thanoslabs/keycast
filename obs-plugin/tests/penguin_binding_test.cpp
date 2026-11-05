#include "input_events.hpp"
#include "penguin_bindings.hpp"
#include "analog_blend.hpp"
#include <cassert>
int main() {
    keycast::InputState input;
    keycast::AnimationState animation;
    input.apply({keycast::InputKind::Key, 0x57, 1});
    input.apply({keycast::InputKind::Key, 0x41, 1});
    input.apply({keycast::InputKind::MouseButton, 1, 1});
    keycast::update_penguin_targets(input, animation);
    animation.tick(0.1f);
    auto &params = animation.parameters();
    assert(params.at("ParamFlipperLeft").value > 0);
    assert(params.at("ParamFlipperRight").value > 0);
    input.apply({keycast::InputKind::Key, 0x57, 0});
    keycast::update_penguin_targets(input, animation);
    animation.tick(0.1f);
    assert(params.at("ParamFlipperLeft").target == 1.0f);
    input.apply({keycast::InputKind::Key, 0x41, 0});
    input.apply({keycast::InputKind::MouseButton, 1, 0});
    keycast::update_penguin_targets(input, animation);
    assert(params.at("ParamFlipperLeft").target == 0.0f);
    assert(keycast::effective_key_depth(input, 0x57, false) == 0.0f);
}
