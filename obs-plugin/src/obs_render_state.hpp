#pragma once
#include "render_lifecycle.hpp"
namespace keycast {
class ObsRenderState {
    SourceDimensions dimensions_;
public:
    bool set_dimensions(SourceDimensions dims) {
        if (!dims.valid()) return false;
        dimensions_ = dims; return true;
    }
    uint32_t width() const { return dimensions_.width; }
    uint32_t height() const { return dimensions_.height; }
};
}
