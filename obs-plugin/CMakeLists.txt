cmake_minimum_required(VERSION 3.28)
project(keycast VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libobs REQUIRED)

add_library(keycast MODULE src/plugin.cpp)
target_link_libraries(keycast PRIVATE OBS::libobs)
set_target_properties(keycast PROPERTIES PREFIX "")
if(WIN32)
  set_target_properties(keycast PROPERTIES OUTPUT_NAME "keycast")
endif()

# Cubism Native SDK is proprietary and is deliberately not bundled here.
# M1 will add an optional, license-compliant SDK integration target.
