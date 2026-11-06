#include <obs-module.h>
#include "model_loader.hpp"
#include "native_cubism_runtime.hpp"
#include "live_penguin.hpp"
#include "obs_render_state.hpp"
#include <memory>
#include <filesystem>
#include <fstream>
#include <string>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("keycast", "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
    return "Keycast: native Live2D avatar source for OBS Studio";
}

namespace {
struct KeycastSource {
    obs_source_t *source = nullptr;
    explicit KeycastSource(obs_source_t *s) : source(s) {}
    std::string model_path;
    bool model_found = false;
    keycast::ModelManifest model;
    keycast::NativeCubismRuntime runtime;
    keycast::LivePenguin penguin;
    keycast::ObsRenderState render_state;
};

const char *source_name(void *) { return "Keycast Live2D"; }

void source_update(void *data, obs_data_t *settings)
{
    auto *state = static_cast<KeycastSource *>(data);
    const char *path = obs_data_get_string(settings, "model_path");
    state->model_path = path ? path : "";
    state->model_found = false;
    state->model = {};
    if (state->model_path.empty()) return;
    std::string error;
    state->model_found = keycast::load_model_manifest(
        std::filesystem::u8path(state->model_path), state->model, error);
    if (!state->model_found)
        blog(LOG_WARNING, "[keycast] Invalid Cubism model: %s", error.c_str());
    else {
        state->runtime.load(std::filesystem::u8path(state->model_path), error);
        blog(LOG_INFO, "[keycast] Cubism assets validated; Core runtime ready: %s", state->runtime.ready() ? "yes" : "no");
    }
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
    if (state && state->model_found)
        state->penguin.tick(seconds, state->runtime);
}

void source_render(void *data, gs_effect_t *)
{
    const auto *state = static_cast<KeycastSource *>(data);
    if (!state || !state->runtime.ready()) return;
    // A platform Cubism mesh renderer is still required. Do not draw a fake
    // placeholder as though it were a functioning Cubism avatar.
}

uint32_t source_width(void *data) {
    const auto *state = static_cast<KeycastSource *>(data);
    return state ? state->render_state.width() : 612;
}
uint32_t source_height(void *data) {
    const auto *state = static_cast<KeycastSource *>(data);
    return state ? state->render_state.height() : 354;
}

obs_properties_t *source_properties(void *)
{
    auto *props = obs_properties_create();
    obs_properties_add_path(props, "model_path", "Avatar .model3.json (Club Penguin or custom)", OBS_PATH_FILE, "Live2D model (*.model3.json)", nullptr);
    obs_properties_add_text(props, "avatar_hint", "Club Penguin: select its exported .model3.json; the original BongOBS .moc3 is not compatible", OBS_TEXT_INFO);
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
    source_info.video_tick = source_tick;
    source_info.video_render = source_render;
    source_info.get_width = source_width;
    source_info.get_height = source_height;
    source_info.get_properties = source_properties;
    obs_register_source(&source_info);
    return true;
}
