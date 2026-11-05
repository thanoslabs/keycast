#pragma once
#include "input_events.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
namespace keycast {
// Polls Windows virtual-key state without logging or persisting key events.
// Elevated applications may not expose input to a non-elevated OBS process.
class WindowsInputPoller {
public:
    void poll(InputState &state) const {
#ifdef _WIN32
        for (uint32_t key : {0x57u, 0x41u, 0x53u, 0x44u}) {
            state.apply({InputKind::Key, key,
                (GetAsyncKeyState(static_cast<int>(key)) & 0x8000) ? 1.0f : 0.0f});
        }
        state.apply({InputKind::MouseButton, 1,
            (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1.0f : 0.0f});
#else
        (void)state;
#endif
    }
};
}
