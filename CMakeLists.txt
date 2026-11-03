cmake_minimum_required(VERSION 3.22.1)

project(radar_vision LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(camera_capture)
add_subdirectory(detector_yolo11)


