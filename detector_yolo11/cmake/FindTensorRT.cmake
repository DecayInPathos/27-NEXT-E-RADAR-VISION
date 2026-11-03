# FindTensorRT.cmake
#
# Locate the TensorRT C++ SDK prepared for this project:
#   ${YOLO11_ENV}/tensorrt-cpp/{include,lib}
#
# Hints (in order): TensorRT_ROOT cache variable, YOLO11_ENV, CONDA_PREFIX,
# then the known yolo11 environment on this machine.
#
# Provides:
#   TensorRT_FOUND
#   TensorRT_INCLUDE_DIRS
#   TensorRT_LIBRARIES
#   TensorRT::nvinfer, TensorRT::nvonnxparser, TensorRT::nvinfer_plugin
#
# Note: the pip/conda TensorRT wheels do not ship C++ headers. See
# docs/yolo11_tensorrt_setup.md for how ${YOLO11_ENV}/tensorrt-cpp was created.

if(NOT TensorRT_ROOT)
    if(DEFINED YOLO11_ENV AND EXISTS "${YOLO11_ENV}/tensorrt-cpp")
        set(TensorRT_ROOT "${YOLO11_ENV}/tensorrt-cpp")
    elseif(DEFINED ENV{CONDA_PREFIX} AND EXISTS "$ENV{CONDA_PREFIX}/tensorrt-cpp")
        set(TensorRT_ROOT "$ENV{CONDA_PREFIX}/tensorrt-cpp")
    elseif(EXISTS "/home/pathos/anaconda3/envs/yolo11/tensorrt-cpp")
        set(TensorRT_ROOT "/home/pathos/anaconda3/envs/yolo11/tensorrt-cpp")
    endif()
endif()

find_path(TensorRT_INCLUDE_DIR
    NAMES NvInfer.h
    HINTS "${TensorRT_ROOT}/include"
    DOC "TensorRT include directory (NvInfer.h)")

find_library(TensorRT_NVINFER_LIB
    NAMES nvinfer
    HINTS "${TensorRT_ROOT}/lib"
    DOC "TensorRT core library")

find_library(TensorRT_NVONNXPARSER_LIB
    NAMES nvonnxparser
    HINTS "${TensorRT_ROOT}/lib"
    DOC "TensorRT ONNX parser library")

find_library(TensorRT_NVINFER_PLUGIN_LIB
    NAMES nvinfer_plugin
    HINTS "${TensorRT_ROOT}/lib"
    DOC "TensorRT plugin library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TensorRT
    REQUIRED_VARS
        TensorRT_INCLUDE_DIR
        TensorRT_NVINFER_LIB
        TensorRT_NVONNXPARSER_LIB
        TensorRT_NVINFER_PLUGIN_LIB)

if(TensorRT_FOUND)
    set(TensorRT_INCLUDE_DIRS "${TensorRT_INCLUDE_DIR}")
    set(TensorRT_LIBRARIES
        "${TensorRT_NVINFER_LIB}"
        "${TensorRT_NVONNXPARSER_LIB}"
        "${TensorRT_NVINFER_PLUGIN_LIB}")

    if(NOT TARGET TensorRT::nvinfer)
        add_library(TensorRT::nvinfer SHARED IMPORTED)
        set_target_properties(TensorRT::nvinfer PROPERTIES
            IMPORTED_LOCATION "${TensorRT_NVINFER_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${TensorRT_INCLUDE_DIR}")

        add_library(TensorRT::nvonnxparser SHARED IMPORTED)
        set_target_properties(TensorRT::nvonnxparser PROPERTIES
            IMPORTED_LOCATION "${TensorRT_NVONNXPARSER_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${TensorRT_INCLUDE_DIR}")

        add_library(TensorRT::nvinfer_plugin SHARED IMPORTED)
        set_target_properties(TensorRT::nvinfer_plugin PROPERTIES
            IMPORTED_LOCATION "${TensorRT_NVINFER_PLUGIN_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${TensorRT_INCLUDE_DIR}")
    endif()
endif()

mark_as_advanced(
    TensorRT_INCLUDE_DIR
    TensorRT_NVINFER_LIB
    TensorRT_NVONNXPARSER_LIB
    TensorRT_NVINFER_PLUGIN_LIB)
