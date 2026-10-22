cmake_minimum_required(VERSION 3.22.1)
project(hik_camera_demo LANGUAGES CXX)

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV 版本：${OpenCV_VERSION}")
message(STATUS "头文件目录：${OpenCV_INCLUDE_DIRS}")
message(STATUS "链接库：${OpenCV_LIBS}")

add_executable(hik_camera_test)
target_compile_features(hik_camera_test PRIVATE cxx_std_17)
target_sources(hik_camera_test PRIVATE "src/main.cpp")

add_library(hik_camera SHARED)
target_sources(hik_camera PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/hik_camera.cpp")
target_include_directories(hik_camera PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                             "/opt/MVS/include"
                                             ${OpenCV_INCLUDE_DIRS})

target_link_libraries(hik_camera PUBLIC "/opt/MVS/lib/64/libMvCameraControl.so" ${OpenCV_LIBS})

target_include_directories(hik_camera_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(hik_camera_test PRIVATE hik_camera ${OpenCV_LIBS})
