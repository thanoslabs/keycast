#pragma once
#include "cubism_runtime.hpp"
#include "model_loader.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
#include <Live2DCubismCore.h>
#endif

namespace keycast {
// Cubism Core owns no file IO; the caller retains this runtime for the full
// lifetime of the source. The SDK must be provided separately under its license.
class NativeCubismRuntime final : public CubismRuntime {
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
    std::vector<unsigned char> moc_bytes_;
    std::vector<unsigned char> model_bytes_;
    csmMoc *moc_ = nullptr;
    csmModel *model_ = nullptr;
#endif
public:
    bool load(const std::filesystem::path &model3, std::string &error) override {
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
        model_ = nullptr;
        moc_ = nullptr;
        model_bytes_.clear();
        moc_bytes_.clear();
        ModelManifest manifest;
        if (!load_model_manifest(model3, manifest, error)) return false;
        std::ifstream stream(manifest.moc, std::ios::binary);
        if (!stream) { error = "Cannot open moc3"; return false; }
        const auto length = std::filesystem::file_size(manifest.moc);
        if (!length || length > 128 * 1024 * 1024) {
            error = "Invalid moc3 size"; return false;
        }
        // Cubism Core requires 64-byte aligned moc and model buffers.
        // std::vector does not guarantee this, so retain aligned backing stores.
        const auto moc_size = static_cast<size_t>(length);
        moc_bytes_.resize(moc_size + 63);
        auto *moc_ptr = reinterpret_cast<void *>(
            (reinterpret_cast<uintptr_t>(moc_bytes_.data()) + 63) & ~uintptr_t(63));
        stream.read(static_cast<char *>(moc_ptr), static_cast<std::streamsize>(moc_size));
        if (!stream) { error = "Failed to read moc3"; return false; }
        moc_ = csmReviveMocInPlace(moc_ptr, static_cast<unsigned int>(moc_size));
        if (!moc_) { error = "Cubism Core rejected moc3"; return false; }
        const auto size = csmGetSizeofModel(moc_);
        if (!size || size > 256 * 1024 * 1024) {
            error = "Invalid Cubism model allocation"; moc_ = nullptr; return false;
        }
        model_bytes_.resize(static_cast<size_t>(size) + 63);
        auto *model_ptr = reinterpret_cast<void *>(
            (reinterpret_cast<uintptr_t>(model_bytes_.data()) + 63) & ~uintptr_t(63));
        model_ = csmInitializeModelInPlace(moc_, model_ptr, size);
        if (!model_) { error = "Cubism model initialization failed"; return false; }
        csmUpdateModel(model_);
        error.clear();
        return true;
#else
        (void)model3;
        error = "Cubism Core SDK is not enabled";
        return false;
#endif
    }
    void set_parameter(const std::string &id, float value) override {
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
        if (!model_) return;
        const auto count = csmGetParameterCount(model_);
        const auto **ids = csmGetParameterIds(model_);
        auto *values = csmGetParameterValues(model_);
        const auto *minimum = csmGetParameterMinimumValues(model_);
        const auto *maximum = csmGetParameterMaximumValues(model_);
        for (int i = 0; i < count; ++i) {
            if (ids[i] && id == ids[i]) {
                values[i] = std::clamp(value, minimum[i], maximum[i]);
                break;
            }
        }
#else
        (void)id; (void)value;
#endif
    }
    void advance(float) override {
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
        if (model_) csmUpdateModel(model_);
#endif
    }
    bool ready() const override {
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
        return model_ != nullptr;
#else
        return false;
#endif
    }
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
    const csmModel *model() const { return model_; }
#endif
};
}
