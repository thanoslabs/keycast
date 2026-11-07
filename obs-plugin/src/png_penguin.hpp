#pragma once
#include <obs-module.h>
#include <graphics/image-file.h>
#include <string>
#include <cmath>
#include <algorithm>
namespace keycast {
struct PngLayer {
    gs_image_file_t image{};
    std::string path;
    bool initialized = false;
    bool loaded = false;
    void release() {
        if (initialized) {
            obs_enter_graphics();
            gs_image_file_free(&image);
            obs_leave_graphics();
        }
        image = {};
        initialized = loaded = false;
    }
    ~PngLayer() { release(); }
    void set(const std::string &next) {
        if (next == path) return;
        release();
        path = next;
        if (path.empty()) return;
        obs_enter_graphics();
        gs_image_file_init(&image, path.c_str());
        initialized = true;
        if (image.loaded) {
            gs_image_file_init_texture(&image);
            loaded = image.texture != nullptr;
        }
        obs_leave_graphics();
    }
    void draw(float x, float y, float opacity = 1.f) const {
        if (!loaded) return;
        gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
        if (!effect) return;
        gs_eparam_t *texture = gs_effect_get_param_by_name(effect, "image");
        if (!texture) return;
        gs_blend_state_push();
        gs_enable_blending(true);
        gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
        gs_matrix_push();
        gs_matrix_translate3f(x, y, 0.f);
        gs_effect_set_texture(texture, image.texture);
        while (gs_effect_loop(effect, "Draw"))
            gs_draw_sprite(image.texture, 0, image.cx, image.cy);
        gs_matrix_pop();
        gs_blend_state_pop();
        (void)opacity;
    }
};
struct PngPenguin {
    PngLayer background, body, left_idle, left_pressed, right_idle, right_pressed;
    PngLayer right_moving;
    PngLayer key_w, key_a, key_s, key_d;
    float left = 0.f, right = 0.f, bounce = 0.f;
    bool keyboard_down = false;
    bool w_down = false, a_down = false, s_down = false, d_down = false;
    bool mouse_motion = false;
    void tick(float dt, bool w, bool a, bool s, bool d, bool right_down, bool moving) {
        w_down = w; a_down = a; s_down = s; d_down = d;
        const bool left_down = w || a || s || d;
        mouse_motion = moving;
        const float blend = 1.f - std::exp(-std::clamp(dt, 0.f, .1f) * 18.f);
        keyboard_down = left_down;
        left += ((left_down ? 1.f : 0.f) - left) * blend;
        right += ((right_down ? 1.f : 0.f) - right) * blend;
        bounce += (((left_down || right_down) ? 1.f : 0.f) - bounce) * blend;
    }
    void render() const {
        background.draw(0, 0);
        if (w_down) key_w.draw(0, 0);
        if (a_down) key_a.draw(0, 0);
        if (s_down) key_s.draw(0, 0);
        if (d_down) key_d.draw(0, 0);
        body.draw(0, -3.f * bounce);
        (left > .5f && left_pressed.loaded ? left_pressed : left_idle).draw(0, -3.f * bounce);
        (right > .5f && right_pressed.loaded ? right_pressed : (mouse_motion && right_moving.loaded ? right_moving : right_idle)).draw(0, -3.f * bounce);
    }
};
}
