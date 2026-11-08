#include <obs-module.h>
#include "obs_render_state.hpp"
#include "png_penguin.hpp"
#include "windows_input.hpp"
#include <string>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("keycast", "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
    return "Keycast: native PNG avatar source for OBS Studio";
}

namespace {
struct KeycastSource {
    obs_source_t *source = nullptr;
    explicit KeycastSource(obs_source_t *s) : source(s) {}
    keycast::ObsRenderState render_state;
    keycast::PngPenguin png;
    keycast::WindowsInputPoller png_poller;
    keycast::InputState png_inputs;
};

const char *source_name(void *) { return "Keycast Avatar"; }

void source_update(void *data, obs_data_t *settings)
{
    auto *state = static_cast<KeycastSource *>(data);
    state->png_poller.set_wheel_mode(static_cast<int>(obs_data_get_int(settings, "wheel_detection_mode")));
    state->png.background.set(obs_data_get_string(settings, "png_background"));
    state->png.body.set(obs_data_get_string(settings, "png_body"));
    state->png.left_idle.set(obs_data_get_string(settings, "png_left_idle"));
    state->png.left_pressed.set(obs_data_get_string(settings, "png_left_pressed"));
    state->png.right_idle.set(obs_data_get_string(settings, "png_right_idle"));
    state->png.right_pressed.set(obs_data_get_string(settings, "png_right_pressed"));
    state->png.set_key_folder(obs_data_get_string(settings, "png_key_folder"));
    state->png.left_arm_max_angle = static_cast<float>(obs_data_get_double(settings, "left_arm_max_angle"));
    state->png.left_arm_pivot_x = static_cast<float>(obs_data_get_double(settings, "left_arm_pivot_x"));
    state->png.left_arm_pivot_y = static_cast<float>(obs_data_get_double(settings, "left_arm_pivot_y"));
    state->png.set_mouse_folder(obs_data_get_string(settings, "png_mouse_folder"));
    state->png.right_moving.set(obs_data_get_string(settings, "png_right_moving"));
    state->png.blink_overlay.set(obs_data_get_string(settings, "png_blink_overlay"));
    state->png.blink_half.set(obs_data_get_string(settings, "png_blink_half"));
}

void source_defaults(obs_data_t *settings)
{
    obs_data_set_default_double(settings, "left_arm_max_angle", 24.0);
    obs_data_set_default_double(settings, "left_arm_pivot_x", 306.0);
    obs_data_set_default_double(settings, "left_arm_pivot_y", 177.0);
    obs_data_set_default_int(settings, "wheel_detection_mode", 0);
}

void *source_create(obs_data_t *settings, obs_source_t *source)
{
    auto *state = new KeycastSource(source);
    source_update(state, settings);
    return state;
}

void source_destroy(void *data)
{
    delete static_cast<KeycastSource *>(data);
}

void source_tick(void *data, float seconds)
{
    auto *state = static_cast<KeycastSource *>(data);
    if (!state) return;
    {
        state->png_poller.poll(state->png_inputs, seconds);
        const bool right = state->png_inputs.amount(keycast::InputKind::MouseButton, 1) > 0;
        const auto motion = state->png_poller.mouse_motion(seconds);
        state->png.tick(seconds, state->png_inputs, right, motion.moving, motion.x, motion.y);
        return;
    }
}

void source_render(void *data, gs_effect_t *)
{
    const auto *state = static_cast<KeycastSource *>(data);
    if (!state) return;
    state->png.render();
}

uint32_t source_width(void *data) {
    const auto *state = static_cast<KeycastSource *>(data);
    return state ? state->render_state.width() : 612;
}
uint32_t source_height(void *data) {
    const auto *state = static_cast<KeycastSource *>(data);
    return state ? state->render_state.height() : 354;
}

bool reload_assets(obs_properties_t *, obs_property_t *, void *data)
{
    auto *state = static_cast<KeycastSource *>(data);
    if (!state) return false;
    state->png.reload();
    return true;
}

bool reset_left_arm(obs_properties_t *, obs_property_t *, void *data)
{
    auto *state = static_cast<KeycastSource *>(data);
    if (!state || !state->source) return false;
    obs_data_t *settings = obs_source_get_settings(state->source);
    obs_data_set_double(settings, "left_arm_max_angle", 24.0);
    obs_data_set_double(settings, "left_arm_pivot_x", 306.0);
    obs_data_set_double(settings, "left_arm_pivot_y", 177.0);
    obs_source_update(state->source, settings);
    obs_data_release(settings);
    return true;
}

obs_properties_t *source_properties(void *)
{
    auto *props = obs_properties_create();
    const char *png_filter = "PNG image (*.png)";
    obs_properties_add_path(props, "png_background", "Background PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_body", "Penguin body PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_left_idle", "Left flipper idle PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_left_pressed", "Left flipper pressed PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_right_idle", "Right flipper idle PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_right_pressed", "Right flipper pressed PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_key_folder", "Key overlay folder (A-Z, 0-9, F1-F24, VK_XX.png for other keys)", OBS_PATH_DIRECTORY, nullptr, nullptr);
    obs_properties_add_float_slider(props, "left_arm_max_angle", "Left arm reach (degrees)", 0.0, 45.0, 1.0);
    obs_properties_add_float_slider(props, "left_arm_pivot_x", "Left arm anchor X", 0.0, 612.0, 1.0);
    obs_properties_add_float_slider(props, "left_arm_pivot_y", "Left arm anchor Y", 0.0, 354.0, 1.0);
    obs_properties_add_button(props, "left_arm_reset", "Reset left arm defaults", reset_left_arm);
    auto *wheel = obs_properties_add_list(props, "wheel_detection_mode", "Mouse wheel detection",
                                          OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(wheel, "Raw Input (recommended)", 0);
    obs_property_list_add_int(wheel, "Low-level hook (compatibility)", 1);
    obs_properties_add_path(props, "png_mouse_folder", "Mouse sprite folder", OBS_PATH_DIRECTORY, nullptr, nullptr);
    obs_properties_add_path(props, "png_right_moving", "Right flipper moving PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_blink_overlay", "Blink (closed eyes) PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_path(props, "png_blink_half", "Blink (half-closed eyes) PNG", OBS_PATH_FILE, png_filter, nullptr);
    obs_properties_add_button(props, "png_reload", "Reload assets", reload_assets);
    return props;
}

obs_source_info source_info = {};
}

bool obs_module_load(void)
{
    source_info.id = "keycast_live2d";
    source_info.type = OBS_SOURCE_TYPE_INPUT;
    source_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
    source_info.get_name = source_name;
    source_info.create = source_create;
    source_info.destroy = source_destroy;
    source_info.update = source_update;
    source_info.get_defaults = source_defaults;
    source_info.video_tick = source_tick;
    source_info.video_render = source_render;
    source_info.get_width = source_width;
    source_info.get_height = source_height;
    source_info.get_properties = source_properties;
    obs_register_source(&source_info);
    return true;
}
