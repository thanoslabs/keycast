#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

namespace keycast {
struct Parameter {
    float value = 0.0f;
    float target = 0.0f;
    float minimum = -1.0f;
    float maximum = 1.0f;
    float response = 12.0f;
    void advance(float dt) {
        dt = std::clamp(dt, 0.0f, 0.1f);
        const float alpha = 1.0f - std::exp(-std::max(0.0f, response) * dt);
        value += (std::clamp(target, minimum, maximum) - value) * alpha;
        value = std::clamp(value, minimum, maximum);
    }
};
class AnimationState {
public:
    void set_target(const std::string &name, float target) {
        auto &p = parameters_[name];
        p.target = target;
    }
    void tick(float dt) {
        for (auto &[_, parameter] : parameters_) parameter.advance(dt);
    }
    const std::unordered_map<std::string, Parameter> &parameters() const { return parameters_; }
private:
    std::unordered_map<std::string, Parameter> parameters_;
};
}
