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
#include "input_events.hpp"
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
    PngLayer right_moving, blink_overlay, blink_half;
    std::mt19937 blink_rng{std::random_device{}()};
    std::uniform_real_distribution<float> blink_interval{2.5f, 6.5f};
    float until_blink = blink_interval(blink_rng);
    float blink_remaining = 0.f;
    static constexpr float blink_duration = 0.20f;
    std::array<PngLayer, 256> key_overlays;
    std::array<bool, 256> key_down{};
    std::string key_folder;
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
    float left = 0.f, right = 0.f, bounce = 0.f;
    bool keyboard_down = false;
    bool mouse_motion = false;
    float hand_x = 0.f, hand_y = 0.f;
    float mousepad_range_x = 14.f, mousepad_range_y = 8.f;
    void tick(float dt, const InputState &inputs, bool right_down, bool moving, float motion_x, float motion_y) {
        bool left_down = false;
        for (unsigned key = 0; key < key_overlays.size(); ++key) {
            key_down[key] = key_overlays[key].loaded && inputs.amount(InputKind::Key, key) > 0.f;
            left_down = left_down || key_down[key];
        }
        mouse_motion = moving;
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
        left += ((left_down ? 1.f : 0.f) - left) * blend;
        right += ((right_down ? 1.f : 0.f) - right) * blend;
        bounce += (((left_down || right_down) ? 1.f : 0.f) - bounce) * blend;
    }
    void render() const {
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
        (left > .5f && left_pressed.loaded ? left_pressed : left_idle).draw(0, -3.f * bounce);
        (right > .5f && right_pressed.loaded ? right_pressed : (mouse_motion && right_moving.loaded ? right_moving : right_idle)).draw(hand_x, hand_y - 3.f * bounce);
    }
};
}
