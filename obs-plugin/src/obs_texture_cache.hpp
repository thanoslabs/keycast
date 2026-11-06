#pragma once
#include "model_manifest.hpp"
#include <obs-module.h>
#include <graphics/image-file.h>
#include <memory>
#include <vector>
namespace keycast {
// Must be initialized and released inside an OBS graphics context.
// This cache is source-owned; it is invalidated when a model changes.
class ObsTextureCache {
    std::vector<std::unique_ptr<gs_image_file_t>> images_;
public:
    ObsTextureCache() = default;
    ObsTextureCache(const ObsTextureCache &) = delete;
    ObsTextureCache &operator=(const ObsTextureCache &) = delete;
    void clear() {
        for (auto &image : images_) gs_image_file_free(image.get());
        images_.clear();
    }
    bool load(const ModelManifest &manifest) {
        clear();
        for (const auto &path : manifest.textures) {
            auto image = std::make_unique<gs_image_file_t>();
            *image = {};
            gs_image_file_init(image.get(), path.u8string().c_str());
            if (!image->loaded) { gs_image_file_free(image.get()); clear(); return false; }
            gs_image_file_init_texture(image.get());
            if (!image->texture) { gs_image_file_free(image.get()); clear(); return false; }
            images_.push_back(std::move(image));
        }
        return !images_.empty();
    }
    gs_texture_t *at(size_t index) const {
        return index < images_.size() ? images_[index]->texture : nullptr;
    }
};
}
