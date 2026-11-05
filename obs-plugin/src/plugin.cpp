#include <obs-module.h>
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
};

const char *source_name(void *) { return "Keycast Live2D"; }

void source_update(void *data, obs_data_t *settings)
{
    auto *state = static_cast<KeycastSource *>(data);
    const char *path = obs_data_get_string(settings, "model_path");
    state->model_path = path ? path : "";
    state->model_found = !state->model_path.empty() &&
        std::filesystem::is_regular_file(std::filesystem::u8path(state->model_path));
    if (!state->model_path.empty() && !state->model_found)
        blog(LOG_WARNING, "[keycast] Model manifest not found: %s", state->model_path.c_str());
    else if (state->model_found)
        blog(LOG_INFO, "[keycast] Model manifest selected: %s (Cubism renderer not installed)", state->model_path.c_str());
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

void source_render(void *, gs_effect_t *)
{
    // M0: intentionally renders nothing. M1 will render Cubism model textures
    // using the OBS graphics context. No HTTP server or browser source.
}

uint32_t source_width(void *) { return 1920; }
uint32_t source_height(void *) { return 1080; }

obs_properties_t *source_properties(void *)
{
    auto *props = obs_properties_create();
    obs_properties_add_path(props, "model_path", "Live2D .model3.json", OBS_PATH_FILE, "Live2D model (*.model3.json)", nullptr);
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
    source_info.video_render = source_render;
    source_info.get_width = source_width;
    source_info.get_height = source_height;
    source_info.get_properties = source_properties;
    obs_register_source(&source_info);
    return true;
}
