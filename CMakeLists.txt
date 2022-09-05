cmake_minimum_required(VERSION 3.13)  # CMake version check
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(jsonPacker)               # Create project "simple_example"

link_directories("./boost")
set(SOURCE
   ${SOURCE}
   ./JsonPacker.cpp
   ./tlv/tlv.cpp
   ./tlv/tlv_box.cpp
)
add_executable(JsonPacker ${SOURCE})
target_include_directories(JsonPacker PUBLIC
                           "path_to_boost/boost"
                           )