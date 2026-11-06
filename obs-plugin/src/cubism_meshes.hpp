#pragma once
#include <cstdint>
#include <vector>
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
#include <Live2DCubismCore.h>
#endif
namespace keycast {
struct CubismMesh {
    int texture_index = -1;
    int render_order = 0;
    bool visible = false;
    float opacity = 0;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<uint16_t> indices;
};
#ifdef KEYCAST_CUBISM_HEADERS_AVAILABLE
inline std::vector<CubismMesh> collect_cubism_meshes(const csmModel *model) {
    std::vector<CubismMesh> meshes;
    if (!model) return meshes;
    const int count = csmGetDrawableCount(model);
    const auto *texture_indices = csmGetDrawableTextureIndices(model);
    const auto *orders = csmGetDrawableRenderOrders(model);
    const auto *opacities = csmGetDrawableOpacities(model);
    const auto *dynamic_flags = csmGetDrawableDynamicFlags(model);
    const auto *vertex_counts = csmGetDrawableVertexCounts(model);
    const auto **positions = csmGetDrawableVertexPositions(model);
    const auto **uvs = csmGetDrawableVertexUvs(model);
    const auto *index_counts = csmGetDrawableIndexCounts(model);
    const auto **indices = csmGetDrawableIndices(model);
    meshes.reserve(count);
    for (int i = 0; i < count; ++i) {
        CubismMesh mesh;
        mesh.texture_index = texture_indices[i];
        mesh.render_order = orders[i];
        mesh.opacity = opacities[i];
        mesh.visible = (dynamic_flags[i] & csmIsVisible) != 0;
        if (vertex_counts[i] > 0 && positions[i] && uvs[i]) {
            mesh.vertices.assign(positions[i], positions[i] + 2 * vertex_counts[i]);
            mesh.uvs.assign(uvs[i], uvs[i] + 2 * vertex_counts[i]);
        }
        if (index_counts[i] > 0 && indices[i])
            mesh.indices.assign(indices[i], indices[i] + index_counts[i]);
        meshes.push_back(std::move(mesh));
    }
    return meshes;
}
#endif
}
