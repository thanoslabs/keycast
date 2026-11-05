#pragma once
#include "input_mapping.hpp"
#include <algorithm>
#include <cstdint>
#include <unordered_map>
namespace keycast {
struct InputEvent {
    InputKind kind = InputKind::Key;
    uint32_t code = 0;
    float amount = 0.0f;
};
class InputState {
public:
    void apply(InputEvent e) {
        const auto key = (static_cast<uint64_t>(e.kind) << 32) | e.code;
        const float amount = std::clamp(e.amount, 0.0f, 1.0f);
        if (amount == 0.0f) active_.erase(key);
        else active_[key] = amount;
    }
    float amount(InputKind kind, uint32_t code) const {
        const auto key = (static_cast<uint64_t>(kind) << 32) | code;
        const auto it = active_.find(key);
        return it == active_.end() ? 0.0f : it->second;
    }
    void clear() { active_.clear(); }
private:
    std::unordered_map<uint64_t, float> active_;
};
}
