#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <string>
namespace keycast {
struct AvatarProfile {
    std::string name = "Club Penguin";
    std::string model_path;
    float animation_response = 12.0f;
};
inline bool load_avatar_profile(const std::filesystem::path &path,
                                AvatarProfile &out, std::string &error) {
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec) ||
        std::filesystem::file_size(path, ec) > 1024 * 1024) {
        error = "Profile missing or too large"; return false;
    }
    std::ifstream in(path);
    auto data = nlohmann::json::parse(in, nullptr, false);
    if (!data.is_object() || !data.value("name", nlohmann::json()).is_string() ||
        !data.value("model", nlohmann::json()).is_string()) {
        error = "Profile must contain string name and model"; return false;
    }
    const auto model = std::filesystem::u8path(data["model"].get<std::string>());
    if (model.is_absolute() || model.empty()) {
        error = "Model path must be relative"; return false;
    }
    out.name = data["name"].get<std::string>();
    out.model_path = model.generic_string();
    error.clear();
    return true;
}
}
