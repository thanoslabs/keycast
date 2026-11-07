#pragma once
#include "input_events.hpp"
#include <algorithm>
#include <cmath>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
namespace keycast {
class WindowsInputPoller {
    mutable bool has_cursor_ = false;
#ifdef _WIN32
    mutable POINT previous_cursor_{};
#endif
    mutable float motion_hold_ = 0.f;
public:
    struct MouseMotion {
        float x = 0.f;
        float y = 0.f;
        bool moving = false;
    };
    MouseMotion mouse_motion(float dt) const {
        float dx = 0.f, dy = 0.f;
#ifdef _WIN32
        POINT point{};
        if (GetCursorPos(&point)) {
            if (has_cursor_) {
                dx = static_cast<float>(point.x - previous_cursor_.x);
                dy = static_cast<float>(point.y - previous_cursor_.y);
                if (dx * dx + dy * dy >= 4.f) motion_hold_ = 0.12f;
            }
            previous_cursor_ = point;
            has_cursor_ = true;
        } else {
            has_cursor_ = false;
        }
#endif
        const bool moving = motion_hold_ > 0.f;
        motion_hold_ = std::max(0.f, motion_hold_ - std::max(0.f, dt));
        // Cursor velocity is bounded so a screen-edge jump cannot fling the hand.
        return {std::clamp(dx / 35.f, -1.f, 1.f),
                std::clamp(dy / 35.f, -1.f, 1.f), moving};
    }
    bool mouse_moving(float dt) const { return mouse_motion(dt).moving; }
    void poll(InputState &state) const {
#ifdef _WIN32
        // Poll the full Windows virtual-key range for configured PNG overlays.
        for (uint32_t key = 0x08u; key <= 0xFEu; ++key)
            state.apply({InputKind::Key, key, (GetAsyncKeyState(static_cast<int>(key)) & 0x8000) ? 1.0f : 0.0f});
        state.apply({InputKind::MouseButton, 1, (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1.0f : 0.0f});
#else
        (void)state;
#endif
    }
};
}
