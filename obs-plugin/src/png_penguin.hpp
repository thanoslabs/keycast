#pragma once
#include <obs-module.h>
#include <graphics/image-file.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <random>
#include <array>
#include <cstdio>
#include <deque>
#include "input_events.hpp"
namespace keycast {
struct PngLayer {
    gs_image_file_t image{};
    std::string path;
    bool initialized = false;
    bool loaded = false;
    inline static thread_local gs_samplerstate_t *active_sampler = nullptr;
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
    void set(const std::string &next, bool force = false) {
        if (!force && next == path) return;
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
    void advance_animation(float dt) {
        if (!loaded || !image.is_animated_gif || dt <= 0.f) return;
        const uint64_t elapsed_ns = static_cast<uint64_t>(std::min(dt, 0.25f) * 1000000000.0);
        if (gs_image_file_tick(&image, elapsed_ns)) {
            obs_enter_graphics();
            gs_image_file_update_texture(&image);
            obs_leave_graphics();
        }
    }
    void draw_rotated(float x, float y, float angle_degrees, float pivot_x, float pivot_y) const {
        if (!loaded) return;
        constexpr float radians = 0.01745329251994329577f;
        gs_matrix_push();
        gs_matrix_translate3f(pivot_x, pivot_y, 0.f);
        gs_matrix_rotaa4f(0.f, 0.f, 1.f, angle_degrees * radians);
        gs_matrix_translate3f(-pivot_x, -pivot_y, 0.f);
        draw(x, y);
        gs_matrix_pop();
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
        gs_matrix_translate3f(std::round(x), std::round(y), 0.f);
        gs_effect_set_texture(texture, image.texture);
        if (active_sampler) gs_effect_set_next_sampler(texture, active_sampler);
        while (gs_effect_loop(effect, "Draw"))
            gs_draw_sprite(image.texture, 0, image.cx, image.cy);
        gs_matrix_pop();
        gs_blend_state_pop();
        (void)opacity;
    }
};
struct PngPenguin {
    bool nearest_filtering = false;
    mutable gs_samplerstate_t *nearest_sampler = nullptr;
    ~PngPenguin() {
        if (nearest_sampler) {
            obs_enter_graphics();
            gs_samplerstate_destroy(nearest_sampler);
            obs_leave_graphics();
        }
    }
    PngLayer background, body, left_idle, left_pressed, right_idle, right_pressed;
    PngLayer right_moving, blink_overlay, blink_half, frenzy_bg;
    std::mt19937 blink_rng{std::random_device{}()};
    std::uniform_real_distribution<float> blink_interval{2.5f, 6.5f};
    float until_blink = blink_interval(blink_rng);
    float blink_remaining = 0.f;
    static constexpr float blink_duration = 0.20f;
    std::array<PngLayer, 256> key_overlays;
    std::array<bool, 256> key_down{};
    std::string key_folder;
    std::string mouse_folder;
    PngLayer mouse_idle, mouse_left, mouse_right, mouse_middle, mouse_m4, mouse_m5;
    unsigned mouse_sprite = 0;
    void reload() {
        background.set(background.path, true);
        body.set(body.path, true);
        left_idle.set(left_idle.path, true);
        left_pressed.set(left_pressed.path, true);
        right_idle.set(right_idle.path, true);
        right_pressed.set(right_pressed.path, true);
        right_moving.set(right_moving.path, true);
        blink_overlay.set(blink_overlay.path, true);
        blink_half.set(blink_half.path, true);
        frenzy_bg.set(frenzy_bg.path, true);
        for (auto *layer : {&mouse_idle, &mouse_left, &mouse_right, &mouse_middle, &mouse_m4, &mouse_m5})
            if (!layer->path.empty()) layer->set(layer->path, true);
        for (auto &layer : key_overlays) {
            if (!layer.path.empty()) layer.set(layer.path, true);
        }
    }
    static std::string key_filename(unsigned key) {
        if ((key >= 0x30 && key <= 0x39) || (key >= 0x41 && key <= 0x5A))
            return std::string(1, static_cast<char>(key)) + ".png";
        if (key >= 0x70 && key <= 0x87)
            return "F" + std::to_string(key - 0x6F) + ".png";
        // Unambiguous virtual-key names for modifiers, punctuation, numpad, etc.
        char filename[16];
        std::snprintf(filename, sizeof(filename), "VK_%02X.png", key);
        return filename;
    }
    void set_key_folder(const std::string &folder) {
        if (folder == key_folder) return;
        key_folder = folder;
        namespace fs = std::filesystem;
        for (unsigned key = 0; key < key_overlays.size(); ++key) {
            const auto file = folder.empty() ? fs::path{} : fs::path(folder) / key_filename(key);
            key_overlays[key].set(!file.empty() && fs::is_regular_file(file) ? file.string() : std::string{});
        }
    }
    void set_mouse_folder(const std::string &folder) {
        if (folder == mouse_folder) return;
        mouse_folder = folder;
        namespace fs = std::filesystem;
        auto assign = [&](PngLayer &layer, const char *filename) {
            const auto file = folder.empty() ? fs::path{} : fs::path(folder) / filename;
            layer.set(!file.empty() && fs::is_regular_file(file) ? file.string() : std::string{});
        };
        assign(mouse_idle, "mouse.png");
        assign(mouse_left, "mouse_left_click.png");
        assign(mouse_right, "mouse_right_click.png");
        assign(mouse_middle, "mouse_middle_click.png");
        assign(mouse_m4, "mouse_m4_click.png");
        assign(mouse_m5, "mouse_m5_click.png");
    }
    float left = 0.f, right = 0.f, bounce = 0.f;
    bool keyboard_down = false;
    int frenzy_threshold = 8;
    float frenzy_clock = 0.f;
    std::deque<float> frenzy_presses;
    std::array<bool, 256> previous_key_pressed{};
    bool frenzy_active = false;
    float left_arm_angle = 0.f;
    float left_arm_max_angle = 24.f;
    float left_arm_pivot_x = 306.f;
    float left_arm_pivot_y = 177.f;
    static float keyboard_reach(unsigned key) {
        // Approximate horizontal reach across the visible left keyboard.
        // Q/A/1 are home; 3/4/W/E reach center; 5/R/T/D/F reach right.
        switch (key) {
            case '1': case 'Q': case 'A': case 'Z': return 0.f;
            case '2': case 'S': case 'X': return 0.20f;
            case '3': case 'W': case 'E': case 'C': return 0.45f;
            case '4': case 'D': case 'V': return 0.68f;
            case '5': case 'R': case 'T': case 'F': case 'G': case 'B': return 1.f;
            default: return -1.f;
        }
    }
    bool mouse_motion = false;
    float hand_x = 0.f, hand_y = 0.f;
    float mousepad_range_x = 14.f, mousepad_range_y = 8.f;
    void tick(float dt, const InputState &inputs, bool right_down, bool moving, float motion_x, float motion_y) {
        frenzy_clock += std::max(0.f, dt);
        bool left_down = false;
        float reach = 0.f;
        for (unsigned key = 0; key < key_overlays.size(); ++key) {
            const bool pressed = inputs.amount(InputKind::Key, key) > 0.f;
            if (pressed && !previous_key_pressed[key] && key >= 0x08 && key <= 0xFE)
                frenzy_presses.push_back(frenzy_clock);
            previous_key_pressed[key] = pressed;
            key_down[key] = key_overlays[key].loaded && pressed;
            left_down = left_down || (inputs.amount(InputKind::Key, key) > 0.f && keyboard_reach(key) >= 0.f);
            if (inputs.amount(InputKind::Key, key) > 0.f)
                reach = std::max(reach, keyboard_reach(key));
        }
        while (!frenzy_presses.empty() && frenzy_clock - frenzy_presses.front() >= 1.f)
            frenzy_presses.pop_front();
        frenzy_active = frenzy_threshold > 0 &&
                        frenzy_presses.size() >= static_cast<size_t>(frenzy_threshold);
        // Advance animated GIF frames only while the frenzy layer is visible.
        // Static PNGs are unaffected; GIFs pause when frenzy ends.
        if (frenzy_active) frenzy_bg.advance_animation(dt);
        mouse_motion = moving;
        mouse_sprite = 0;
        // A single mouse PNG per frame; deterministic priority for combinations.
        if (inputs.amount(InputKind::MouseButton, 5) > 0.f) mouse_sprite = 5;
        if (inputs.amount(InputKind::MouseButton, 4) > 0.f) mouse_sprite = 4;
        if (inputs.amount(InputKind::MouseButton, 2) > 0.f) mouse_sprite = 2;
        if (inputs.amount(InputKind::MouseButton, 3) > 0.f ||
            inputs.amount(InputKind::MouseButton, 6) > 0.f) mouse_sprite = 3;
        if (inputs.amount(InputKind::MouseButton, 1) > 0.f) mouse_sprite = 1;
        // Bounded elliptical travel; decay to the resting pose when idle.
        const float step = std::clamp(dt, 0.f, .1f);
        hand_x += std::clamp(motion_x, -1.f, 1.f) * mousepad_range_x * 0.30f;
        hand_y += std::clamp(motion_y, -1.f, 1.f) * mousepad_range_y * 0.30f;
        if (!moving) {
            const float recovery = std::exp(-step * 7.f);
            hand_x *= recovery;
            hand_y *= recovery;
        }
        const float nx = hand_x / std::max(1.f, mousepad_range_x);
        const float ny = hand_y / std::max(1.f, mousepad_range_y);
        const float radius = std::sqrt(nx * nx + ny * ny);
        if (radius > 1.f) {
            hand_x /= radius;
            hand_y /= radius;
        }
        const float elapsed = std::clamp(dt, 0.f, 0.1f);
        if (blink_remaining > 0.f) {
            blink_remaining = std::max(0.f, blink_remaining - elapsed);
        } else if (blink_overlay.loaded) {
            until_blink -= elapsed;
            if (until_blink <= 0.f) {
                blink_remaining = blink_duration;
                until_blink = blink_interval(blink_rng);
            }
        }
        const float blend = 1.f - std::exp(-std::clamp(dt, 0.f, .1f) * 18.f);
        keyboard_down = left_down;
        const float target_angle = left_down ? reach * left_arm_max_angle : 0.f;
        left_arm_angle += (target_angle - left_arm_angle) * blend;
        left += ((left_down ? 1.f : 0.f) - left) * blend;
        right += ((right_down ? 1.f : 0.f) - right) * blend;
        bounce += (((left_down || right_down) ? 1.f : 0.f) - bounce) * blend;
    }
    void render() const {
        // Nearest-neighbor sampling avoids interpolating transparent edge RGB into
        // adjacent layers. Use the OBS default linear sampler when disabled.
        if (nearest_filtering && !nearest_sampler) {
            gs_sampler_info info{};
            info.filter = GS_FILTER_POINT;
            info.address_u = GS_ADDRESS_CLAMP;
            info.address_v = GS_ADDRESS_CLAMP;
            info.address_w = GS_ADDRESS_CLAMP;
            nearest_sampler = gs_samplerstate_create(&info);
        }
        PngLayer::active_sampler = nearest_filtering ? nearest_sampler : nullptr;
        if (frenzy_active) frenzy_bg.draw(0, 0);
        background.draw(0, 0);
        body.draw(0, -3.f * bounce);
        if (blink_remaining > 0.f) {
            const float progress = 1.f - blink_remaining / blink_duration;
            if (blink_half.loaded) {
                if (progress >= 0.10f && progress < 0.35f)
                    blink_half.draw(0, -3.f * bounce);
                else if (progress >= 0.35f && progress < 0.65f)
                    blink_overlay.draw(0, -3.f * bounce);
                else if (progress >= 0.65f && progress < 0.90f)
                    blink_half.draw(0, -3.f * bounce);
            } else {
                blink_overlay.draw(0, -3.f * bounce);
            }
        }
        for (unsigned key = 0; key < key_overlays.size(); ++key)
            if (key_down[key]) key_overlays[key].draw(0, 0);
        (left > .5f && left_pressed.loaded ? left_pressed : left_idle).draw_rotated(0, -3.f * bounce, left_arm_angle, left_arm_pivot_x, left_arm_pivot_y);
        // Mouse and right flipper share the same bounded movement transform.
        // Draw mouse first so the flipper appears naturally on top of it.
        mouse_idle.draw(hand_x, hand_y - 3.f * bounce);
        const PngLayer *overlay = nullptr;
        if (mouse_sprite == 1 && mouse_left.loaded) overlay = &mouse_left;
        else if (mouse_sprite == 2 && mouse_right.loaded) overlay = &mouse_right;
        else if (mouse_sprite == 3 && mouse_middle.loaded) overlay = &mouse_middle;
        else if (mouse_sprite == 4 && mouse_m4.loaded) overlay = &mouse_m4;
        else if (mouse_sprite == 5 && mouse_m5.loaded) overlay = &mouse_m5;
        if (overlay) overlay->draw(hand_x, hand_y - 3.f * bounce);
        (right > .5f && right_pressed.loaded ? right_pressed : (mouse_motion && right_moving.loaded ? right_moving : right_idle)).draw(hand_x, hand_y - 3.f * bounce);
        PngLayer::active_sampler = nullptr;
    }
};
}
