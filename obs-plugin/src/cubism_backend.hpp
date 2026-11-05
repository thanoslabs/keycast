#pragma once
#include "cubism_runtime.hpp"
#include "model_loader.hpp"
#include <unordered_map>
namespace keycast {
// Validates a Cubism export and stages parameter values. A native SDK adapter
// must replace this class before model deformation or drawing can work.
class StagedCubismBackend final : public CubismRuntime {
    ModelManifest manifest_;
    std::unordered_map<std::string, float> parameters_;
    bool loaded_ = false;
public:
    bool load(const std::filesystem::path &model3, std::string &error) override {
        loaded_ = load_model_manifest(model3, manifest_, error);
        parameters_.clear();
        return loaded_;
    }
    void set_parameter(const std::string &id, float value) override {
        if (loaded_) parameters_[id] = value;
    }
    void advance(float) override {}
    bool ready() const override { return false; } // No Cubism SDK model is instantiated.
    bool assets_validated() const { return loaded_; }
    const ModelManifest &manifest() const { return manifest_; }
};
}
