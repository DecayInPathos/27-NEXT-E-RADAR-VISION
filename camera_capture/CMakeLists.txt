cmake_minimum_required(VERSION 3.22.1)

project(hik_camera_demo LANGUAGES CXX)

add_executable(hik_camera)

target_sources(hik_camera PRIVATE "src/main.cpp")

target_include_directories(hik_camera PUBLIC "/opt/MVS/include")

target_link_libraries(hik_camera PRIVATE "/opt/MVS/lib/64/libMvCameraControl.so")
