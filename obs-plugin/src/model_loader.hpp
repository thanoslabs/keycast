#pragma once
#include "model_manifest.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

namespace keycast {
inline bool load_model_manifest(const std::filesystem::path &path, ModelManifest &model,
                                std::string &error)
{
    model = {};
    error.clear();
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec) ||
        std::filesystem::file_size(path, ec) > 4 * 1024 * 1024) {
        error = "Model manifest missing or exceeds 4 MiB"; return false;
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) { error = "Cannot open model manifest"; return false; }
    auto json = nlohmann::json::parse(input, nullptr, false);
    if (json.is_discarded() || !json.is_object() ||
        !json.contains("FileReferences") || !json["FileReferences"].is_object()) {
        error = "Invalid Cubism model3 JSON or missing FileReferences"; return false;
    }
    const auto &refs = json["FileReferences"];
    auto resolve = [&](const std::string &name, std::filesystem::path &out) {
        out = resolve_asset(path, name);
        if (out.empty() || !std::filesystem::is_regular_file(out, ec)) {
            error = "Missing or unsafe model asset: " + name;
            return false;
        }
        return true;
    };
    if (!refs.contains("Moc") || !refs["Moc"].is_string() ||
        !resolve(refs["Moc"].get<std::string>(), model.moc)) return false;
    if (!refs.contains("Textures") || !refs["Textures"].is_array() ||
        refs["Textures"].empty() || refs["Textures"].size() > 128) {
        error = "Expected 1-128 model textures"; return false;
    }
    for (const auto &item : refs["Textures"]) {
        if (!item.is_string()) { error = "Texture path is not a string"; return false; }
        std::filesystem::path asset;
        if (!resolve(item.get<std::string>(), asset)) return false;
        model.textures.push_back(std::move(asset));
    }
    auto optional = [&](const char *key, std::vector<std::filesystem::path> &out) {
        if (!refs.contains(key)) return true;
        if (!refs[key].is_string()) { error = std::string("Invalid ") + key; return false; }
        std::filesystem::path asset;
        if (!resolve(refs[key].get<std::string>(), asset)) return false;
        out.push_back(std::move(asset));
        return true;
    };
    if (!optional("Physics", model.physics)) return false;
    if (refs.contains("Expressions")) {
        if (!refs["Expressions"].is_array()) { error = "Invalid Expressions"; return false; }
        for (const auto &entry : refs["Expressions"]) {
            if (!entry.is_object() || !entry.contains("File") || !entry["File"].is_string()) {
                error = "Invalid expression entry"; return false;
            }
            std::filesystem::path asset;
            if (!resolve(entry["File"].get<std::string>(), asset)) return false;
            model.expressions.push_back(std::move(asset));
        }
    }
    if (refs.contains("Motions")) {
        if (!refs["Motions"].is_object()) { error = "Invalid Motions"; return false; }
        for (auto it = refs["Motions"].begin(); it != refs["Motions"].end(); ++it) {
            if (!it.value().is_array()) { error = "Invalid motion group"; return false; }
            for (const auto &entry : it.value()) {
                if (!entry.is_object() || !entry.contains("File") || !entry["File"].is_string()) {
                    error = "Invalid motion entry"; return false;
                }
                std::filesystem::path asset;
                if (!resolve(entry["File"].get<std::string>(), asset)) return false;
                model.motions.push_back(std::move(asset));
            }
        }
    }
    model.manifest = path;
    return true;
}
}
