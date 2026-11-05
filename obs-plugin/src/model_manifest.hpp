#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace keycast {
struct ModelManifest {
    std::filesystem::path manifest;
    std::filesystem::path moc;
    std::vector<std::filesystem::path> textures;
    std::vector<std::filesystem::path> physics;
    std::vector<std::filesystem::path> expressions;
    std::vector<std::filesystem::path> motions;
};
inline bool within_model_dir(const std::filesystem::path &root, const std::filesystem::path &candidate)
{
    const auto base = std::filesystem::weakly_canonical(root);
    const auto full = std::filesystem::weakly_canonical(candidate);
    auto a = base.begin(), b = full.begin();
    for (; a != base.end(); ++a, ++b) {
        if (b == full.end() || *a != *b) return false;
    }
    return true;
}
inline std::filesystem::path resolve_asset(const std::filesystem::path &manifest,
                                            const std::string &relative)
{
    if (relative.empty()) return {};
    const auto root = manifest.parent_path();
    const auto asset = root / std::filesystem::u8path(relative);
    if (!within_model_dir(root, asset)) return {};
    return asset;
}
}
