#pragma once
#include "render_lifecycle.hpp"
#include <cstdint>
namespace keycast {
// Frame bookkeeping stays independent of OBS graphics APIs for unit tests.
// A real mesh/texture renderer must implement RenderLifecycle separately.
class ObsRenderState {
    SourceDimensions dimensions_;
    bool graphics_ready_ = false;
public:
    explicit ObsRenderState(SourceDimensions dims = {}) : dimensions_(dims) {}
    bool set_dimensions(SourceDimensions dims) {
        if (!dims.valid()) return false;
        dimensions_ = dims;
        return true;
    }
    void set_graphics_ready(bool ready) { graphics_ready_ = ready; }
    bool graphics_ready() const { return graphics_ready_; }
    uint32_t width() const { return dimensions_.width; }
    uint32_t height() const { return dimensions_.height; }
};
}
