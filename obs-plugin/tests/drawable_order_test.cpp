#include "drawable_order.hpp"
#include "penguin_bindings.hpp"
#include "analog_blend.hpp"
#include <cassert>
int main() {
    std::vector<keycast::CubismMesh> meshes(4);
    for (auto &mesh : meshes) {
        mesh.visible = true; mesh.opacity = 1;
        mesh.texture_index = 0; mesh.vertices = {0,0,1,0,1,1};
        mesh.uvs = mesh.vertices; mesh.indices = {0,1,2};
    }
    meshes[0].render_order = 3;
    meshes[1].render_order = 1;
    meshes[2].render_order = 1;
    meshes[3].visible = false;
    const auto order = keycast::drawable_order(meshes);
    assert((order == std::vector<size_t>{1,2,0}));
    keycast::InputState inputs;
    inputs.apply({keycast::InputKind::Key, 0x57, 1});
    inputs.apply({keycast::InputKind::Key, 0x41, 1});
    keycast::AnimationState animation;
    keycast::update_penguin_targets(inputs, animation);
    animation.tick(0.05f);
    assert(animation.parameters().at("ParamFlipperLeft").value > 0);
    inputs.apply({keycast::InputKind::Key, 0x57, 0});
    keycast::update_penguin_targets(inputs, animation);
    assert(animation.parameters().at("ParamFlipperLeft").target == 1);
}
