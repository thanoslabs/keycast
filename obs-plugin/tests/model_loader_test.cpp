#include "model_loader.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

static void write(const fs::path &path, const std::string &contents) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    if (!out || !(out << contents)) throw std::runtime_error("Cannot write test fixture");
}

static void expect(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    const auto root = fs::temp_directory_path() /
        ("keycast-manifest-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root / "model");
    try {
        const auto manifest = root / "model" / "avatar.model3.json";
        write(root / "model" / "avatar.moc3", "dummy");
        write(root / "model" / "tex.png", "dummy");
        write(root / "model" / "physics.json", "{}");
        write(root / "model" / "expressions" / "smile.exp3.json", "{}");
        write(root / "model" / "motions" / "idle.motion3.json", "{}");
        const std::string valid = R"({"FileReferences":{"Moc":"avatar.moc3","Textures":["tex.png"],"Physics":"physics.json","Expressions":[{"File":"expressions/smile.exp3.json"}],"Motions":{"Idle":[{"File":"motions/idle.motion3.json"}]}}})";
        keycast::ModelManifest model;
        std::string error;
        auto check = [&](const std::string &json, bool expected, const char *description) {
            write(manifest, json);
            const bool ok = keycast::load_model_manifest(manifest, model, error);
            expect(ok == expected, description);
            if (!expected) expect(!error.empty(), "Failure must explain the reason");
        };
        check(valid, true, "Valid manifest should load");
        expect(model.textures.size() == 1 && model.physics.size() == 1 &&
               model.expressions.size() == 1 && model.motions.size() == 1,
               "Optional assets should be parsed");
        check("{", false, "Malformed JSON must fail");
        check("{}", false, "Missing FileReferences must fail");
        check(R"({"FileReferences":{"Moc":"avatar.moc3","Textures":[]}})", false,
              "Empty textures must fail");
        check(R"({"FileReferences":{"Moc":"missing.moc3","Textures":["tex.png"]}})", false,
              "Missing moc must fail");
        check(R"({"FileReferences":{"Moc":"avatar.moc3","Textures":["missing.png"]}})", false,
              "Missing texture must fail");
        write(root / "outside.moc3", "dummy");
        check(R"({"FileReferences":{"Moc":"../outside.moc3","Textures":["tex.png"]}})", false,
              "Parent traversal must fail");
        check(R"({"FileReferences":{"Moc":"avatar.moc3","Textures":["/etc/passwd"]}})", false,
              "Absolute asset paths must fail");
        check(R"({"FileReferences":{"Moc":"avatar.moc3","Textures":["tex.png"],"Motions":{"Idle":[{"File":"missing.motion3.json"}]}}})", false,
              "Missing motion must fail");
        check(R"({"FileReferences":{"Moc":"avatar.moc3","Textures":["tex.png"],"Expressions":[{"File":5}]}})", false,
              "Malformed expression must fail");
        check(valid, true, "Successful load after failure should reset state");
        expect(model.textures.size() == 1, "No stale assets may remain");
        fs::remove_all(root);
        std::cout << "Model manifest tests passed\n";
        return 0;
    } catch (const std::exception &e) {
        fs::remove_all(root);
        std::cerr << "Model manifest tests failed: " << e.what() << '\n';
        return 1;
    }
}
