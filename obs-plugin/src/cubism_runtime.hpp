#pragma once
#include <filesystem>
#include <string>

namespace keycast {
// The Cubism SDK is not redistributable through this repository. This interface
// keeps manifest validation and OBS source lifetime independent of SDK internals.
struct CubismRuntime {
    virtual ~CubismRuntime() = default;
    virtual bool load(const std::filesystem::path &model3, std::string &error) = 0;
    virtual void set_parameter(const std::string &id, float value) = 0;
    virtual void advance(float seconds) = 0;
    virtual bool ready() const = 0;
};
// An explicit unsupported implementation avoids accidentally reporting a loaded
// model when the proprietary Cubism runtime is unavailable.
class UnavailableCubismRuntime final : public CubismRuntime {
public:
    bool load(const std::filesystem::path &, std::string &error) override {
        error = "Cubism Native SDK backend not configured"; return false;
    }
    void set_parameter(const std::string &, float) override {}
    void advance(float) override {}
    bool ready() const override { return false; }
};
}
