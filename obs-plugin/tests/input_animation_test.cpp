#include "animation.hpp"
#include "input_mapping.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

static void require(bool ok, const char *message) {
    if (!ok) throw std::runtime_error(message);
}
static float value(const keycast::AnimationState &state, const char *name) {
    const auto &p = state.parameters();
    auto it = p.find(name);
    require(it != p.end(), "Missing parameter");
    return it->second.value;
}
int main() {
    try {
        keycast::AnimationState state;
        keycast::InputMapper mapper;
        mapper.add({keycast::InputKind::Key, 0x57, "ParamFlipperLeft", 1.0f, 0.0f});
        mapper.add({keycast::InputKind::MouseButton, 1, "ParamFlipperRight", 1.0f, 0.0f});
        mapper.add({keycast::InputKind::Analog, 0x57, "ParamKeyDepth", 1.0f, 0.0f});
        mapper.apply(keycast::InputKind::Key, 0x57, 1.0f, state);
        mapper.apply(keycast::InputKind::MouseButton, 1, 1.0f, state);
        state.tick(0.05f);
        require(value(state, "ParamFlipperLeft") > 0.0f, "Keyboard press should animate");
        require(value(state, "ParamFlipperRight") > 0.0f, "Simultaneous mouse press should animate");
        mapper.apply(keycast::InputKind::Analog, 0x57, 0.4f, state);
        state.tick(0.05f);
        require(value(state, "ParamKeyDepth") > 0.0f && value(state, "ParamKeyDepth") < 0.4f,
                "Analog depth should interpolate");
        mapper.apply(keycast::InputKind::Key, 0x57, 0.0f, state);
        mapper.apply(keycast::InputKind::MouseButton, 1, 0.0f, state);
        for (int i = 0; i < 40; ++i) state.tick(0.05f);
        require(std::abs(value(state, "ParamFlipperLeft")) < 0.001f,
                "Keyboard release should return to idle");
        require(std::abs(value(state, "ParamFlipperRight")) < 0.001f,
                "Mouse release should return to idle");
        mapper.apply(keycast::InputKind::Analog, 0x57, 2.0f, state);
        state.tick(0.1f);
        require(value(state, "ParamKeyDepth") <= 1.0f, "Analog depth must clamp");
        std::cout << "Input animation tests passed\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Input animation tests failed: " << e.what() << '\n';
        return 1;
    }
}
