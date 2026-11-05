cmake_minimum_required(VERSION 3.28)
project(keycast VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libobs REQUIRED)
find_package(nlohmann_json 3.10 REQUIRED)

add_library(keycast MODULE src/plugin.cpp)
target_link_libraries(keycast PRIVATE OBS::libobs nlohmann_json::nlohmann_json)
set_target_properties(keycast PROPERTIES PREFIX "")
if(WIN32)
  set_target_properties(keycast PROPERTIES OUTPUT_NAME "keycast")
endif()

# Cubism Native SDK is proprietary and is deliberately not bundled here.
# M1 will add an optional, license-compliant SDK integration target.

include(CTest)
if(BUILD_TESTING)
  add_executable(keycast-model-loader-test tests/model_loader_test.cpp)
  target_include_directories(keycast-model-loader-test PRIVATE src)
  target_link_libraries(keycast-model-loader-test PRIVATE nlohmann_json::nlohmann_json)
  add_test(NAME model-manifest-validation COMMAND keycast-model-loader-test)
endif()
