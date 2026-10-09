cmake_minimum_required(VERSION 3.22.1)

project(hik_camera_demo LANGUAGES CXX)

add_executable(hik_camera_test)

target_compile_features(hik_camera_test PRIVATE cxx_std_17)

target_sources(hik_camera_test PRIVATE "src/main.cpp")

add_library(hik_camera SHARED)

target_sources(hik_camera PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/hik_camera.cpp")

target_include_directories(hik_camera PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                             "/opt/MVS/include")

target_link_libraries(hik_camera PUBLIC "/opt/MVS/lib/64/libMvCameraControl.so")

target_include_directories(hik_camera_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(hik_camera_test PRIVATE hik_camera)
