#pragma once
#include "cubism_meshes.hpp"
#include <algorithm>
#include <numeric>
#include <vector>
namespace keycast {
// Cubism drawables must be rendered back-to-front; a stable sort preserves
// deterministic order for ties and avoids frame-to-frame flicker.
inline std::vector<size_t> drawable_order(const std::vector<CubismMesh> &meshes) {
    std::vector<size_t> indices(meshes.size());
    std::iota(indices.begin(), indices.end(), size_t{0});
    std::stable_sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return meshes[a].render_order < meshes[b].render_order;
    });
    indices.erase(std::remove_if(indices.begin(), indices.end(), [&](size_t i) {
        const auto &mesh = meshes[i];
        return !mesh.visible || mesh.opacity <= 0.0f ||
            mesh.texture_index < 0 || mesh.vertices.empty() ||
            mesh.uvs.size() != mesh.vertices.size() || mesh.indices.empty();
    }), indices.end());
    return indices;
}
}
