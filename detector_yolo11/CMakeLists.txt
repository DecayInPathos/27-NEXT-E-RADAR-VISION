cmake_minimum_required(VERSION 3.22.1)

project(detector_yolo11)
enable_language(CXX)
enable_language(CUDA)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CUDA_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#----------------CUDA环境链接库RTX5060----------------
find_package(CUDAToolkit REQUIRED)
set(CMAKE_CUDA_ARCHITECTURES 120)
add_library(cuda_env INTERFACE)
target_link_libraries(cuda_env INTERFACE CUDA::cudart)

#----------------tenorRT环境链接库RTX5060----------------
find_package(TensorRT REQUIRED)
add_library(tensorrt_env INTERFACE)
target_link_libraries(tensorrt_env INTERFACE
        TensorRT::nvinfer
        TensorRT::nvonnxparser
        TensorRT::nvinfer_plugin)

#----------------OpenCV环境链接库RTX5060----------------
find_package(OpenCV REQUIRED)
add_library(opencv_env INTERFACE)
target_link_libraries(opencv_env INTERFACE ${OpenCV_LIBS})
target_include_directories(opencv_env INTERFACE ${OpenCV_INCLUDE_DIRS})

#----------------NPP环境链接库RTX5060----------------
find_package(NPP REQUIRED)
add_library(npp_env INTERFACE)
target_link_libraries(npp_env INTERFACE NPP::nppc NPP::nppial NPP::nppisu)

#----------------preprocess gpu预先处理库----------------
add_library(preprocess_gpu STATIC)
target_sources(preprocess_gpu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/yolo_detctor.cu")
target_include_directories(preprocess_gpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(preprocess_gpu PUBLIC opencv_env  npp_env tensorrt_env cuda_env)
