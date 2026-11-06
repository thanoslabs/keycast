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

# Cubism Core is licensed separately. Require an explicitly supplied SDK root;
# never download or commit the proprietary binaries.
option(KEYCAST_WITH_CUBISM "Enable Cubism Native SDK integration" OFF)
set(KEYCAST_CUBISM_SDK_ROOT "" CACHE PATH "Path to locally installed Cubism SDK for Native")
if(KEYCAST_WITH_CUBISM)
  if(NOT KEYCAST_CUBISM_SDK_ROOT OR NOT EXISTS "${KEYCAST_CUBISM_SDK_ROOT}/Core/include/Live2DCubismCore.h")
    message(FATAL_ERROR "KEYCAST_WITH_CUBISM requires a Cubism SDK for Native root with Core/include/Live2DCubismCore.h")
  endif()
  # Core API availability is verified here. Runtime allocation and mesh drawing
  # still require a platform adapter and compatible Core library selection.
  target_include_directories(keycast PRIVATE "${KEYCAST_CUBISM_SDK_ROOT}/Core/include")
  target_compile_definitions(keycast PRIVATE KEYCAST_CUBISM_HEADERS_AVAILABLE=1)
  set(KEYCAST_CUBISM_CORE_LIBRARY "" CACHE FILEPATH "Licensed Cubism Core library for target platform")
  if(NOT KEYCAST_CUBISM_CORE_LIBRARY OR NOT EXISTS "${KEYCAST_CUBISM_CORE_LIBRARY}")
    message(FATAL_ERROR "KEYCAST_WITH_CUBISM requires KEYCAST_CUBISM_CORE_LIBRARY")
  endif()
  target_link_libraries(keycast PRIVATE "${KEYCAST_CUBISM_CORE_LIBRARY}")
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(keycast-model-loader-test tests/model_loader_test.cpp)
  target_include_directories(keycast-model-loader-test PRIVATE src)
  target_link_libraries(keycast-model-loader-test PRIVATE nlohmann_json::nlohmann_json)
  add_test(NAME model-manifest-validation COMMAND keycast-model-loader-test)
  add_executable(keycast-input-animation-test tests/input_animation_test.cpp)
  target_include_directories(keycast-input-animation-test PRIVATE src)
  add_test(NAME input-animation COMMAND keycast-input-animation-test)
  add_executable(keycast-penguin-binding-test tests/penguin_binding_test.cpp)
  target_include_directories(keycast-penguin-binding-test PRIVATE src)
  add_test(NAME penguin-bindings COMMAND keycast-penguin-binding-test)
endif()
