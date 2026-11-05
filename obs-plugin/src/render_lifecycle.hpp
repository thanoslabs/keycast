#pragma once
#include <cstdint>
namespace keycast {
// OBS graphics resources must be created/destroyed with the OBS graphics
// context active. The renderer backend owns texture and mesh allocation.
struct RenderLifecycle {
    virtual ~RenderLifecycle() = default;
    virtual bool initialize_graphics() = 0;
    virtual void release_graphics() = 0;
    virtual void render_frame(uint32_t width, uint32_t height) = 0;
};
struct SourceDimensions {
    uint32_t width = 612;
    uint32_t height = 354;
    bool valid() const { return width > 0 && height > 0 && width <= 8192 && height <= 8192; }
};
}
