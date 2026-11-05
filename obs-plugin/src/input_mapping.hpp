#pragma once
#include "animation.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace keycast {
enum class InputKind { Key, MouseButton, MouseWheel, Analog };
struct Binding {
    InputKind kind = InputKind::Key;
    uint32_t code = 0;
    std::string parameter;
    float pressed_value = 1.0f;
    float released_value = 0.0f;
};
class InputMapper {
public:
    void add(Binding binding) { bindings_.push_back(std::move(binding)); }
    void apply(InputKind kind, uint32_t code, float amount, AnimationState &animation) const {
        for (const auto &binding : bindings_) {
            if (binding.kind != kind || binding.code != code) continue;
            const float t = std::clamp(amount, 0.0f, 1.0f);
            animation.set_target(binding.parameter,
                binding.released_value + t * (binding.pressed_value - binding.released_value));
        }
    }
private:
    std::vector<Binding> bindings_;
};
}
