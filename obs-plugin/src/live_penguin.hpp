#pragma once
#include "windows_input.hpp"
#include "penguin_bindings.hpp"
#include "cubism_runtime.hpp"
namespace keycast {
class LivePenguin {
    WindowsInputPoller poller_;
    InputState inputs_;
    AnimationState animation_;
public:
    void tick(float dt, CubismRuntime &runtime) {
        poller_.poll(inputs_);
        update_penguin_targets(inputs_, animation_);
        animation_.tick(dt);
        for (const auto &entry : animation_.parameters())
            runtime.set_parameter(entry.first, entry.second.value);
        runtime.advance(dt);
    }
    const AnimationState &animation() const { return animation_; }
};
}
