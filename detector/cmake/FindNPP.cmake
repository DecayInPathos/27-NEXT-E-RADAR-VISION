# FindNPP.cmake
#
# Locate NVIDIA NPP (Performance Primitives) from the yolo11 conda environment.
#
# Hints (in order): NPP_ROOT, YOLO11_ENV, CONDA_PREFIX, then the known yolo11
# environment on this machine. NPP 13.1 installs headers and libraries under:
#   <prefix>/targets/x86_64-linux/{include,lib}
#
# Provides:
#   NPP_FOUND
#   NPP_INCLUDE_DIRS
#   NPP_LIBRARIES          (nppc + nppial + nppisu, as used by this project)
#   NPP_NPPIG_LIBRARY      (geometry/resize, find_library result if available)
#   NPP_NPPIDEI_LIBRARY    (data exchange/init, if available)
#   NPP::nppc, NPP::nppial, NPP::nppisu
#
# Note: NPP 13.1 no longer ships a libnppi.so umbrella. Link only the component
# libraries you use (nppc is required by every NPP program).

if(NOT NPP_ROOT)
    if(DEFINED YOLO11_ENV AND EXISTS "${YOLO11_ENV}/targets/x86_64-linux")
        set(NPP_ROOT "${YOLO11_ENV}/targets/x86_64-linux")
    elseif(DEFINED ENV{CONDA_PREFIX} AND EXISTS "$ENV{CONDA_PREFIX}/targets/x86_64-linux")
        set(NPP_ROOT "$ENV{CONDA_PREFIX}/targets/x86_64-linux")
    elseif(EXISTS "/home/pathos/anaconda3/envs/yolo11/targets/x86_64-linux")
        set(NPP_ROOT "/home/pathos/anaconda3/envs/yolo11/targets/x86_64-linux")
    endif()
endif()

find_path(NPP_INCLUDE_DIR
    NAMES npp.h
    HINTS "${NPP_ROOT}/include"
    DOC "NPP include directory (npp.h)")

find_library(NPP_NPPC_LIBRARY
    NAMES nppc
    HINTS "${NPP_ROOT}/lib"
    DOC "NPP core library")

find_library(NPP_NPPIAL_LIBRARY
    NAMES nppial
    HINTS "${NPP_ROOT}/lib"
    DOC "NPP arithmetic/logical library")

find_library(NPP_NPPISU_LIBRARY
    NAMES nppisu
    HINTS "${NPP_ROOT}/lib"
    DOC "NPP support library")

# Optional components commonly used for preprocessing (resize, color conversion).
find_library(NPP_NPPIG_LIBRARY   NAMES nppig   HINTS "${NPP_ROOT}/lib")
find_library(NPP_NPPIDEI_LIBRARY NAMES nppidei HINTS "${NPP_ROOT}/lib")
find_library(NPP_NPPICC_LIBRARY  NAMES nppicc  HINTS "${NPP_ROOT}/lib")
find_library(NPP_NPPIF_LIBRARY   NAMES nppif   HINTS "${NPP_ROOT}/lib")
find_library(NPP_NPPS_LIBRARY    NAMES npps    HINTS "${NPP_ROOT}/lib")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NPP
    REQUIRED_VARS
        NPP_INCLUDE_DIR
        NPP_NPPC_LIBRARY
        NPP_NPPIAL_LIBRARY
        NPP_NPPISU_LIBRARY)

if(NPP_FOUND)
    set(NPP_INCLUDE_DIRS "${NPP_INCLUDE_DIR}")
    set(NPP_LIBRARIES
        "${NPP_NPPC_LIBRARY}"
        "${NPP_NPPIAL_LIBRARY}"
        "${NPP_NPPISU_LIBRARY}")

    if(NOT TARGET NPP::nppc)
        add_library(NPP::nppc SHARED IMPORTED
                ../src/yolov11n_detctor.cpp)
        set_target_properties(NPP::nppc PROPERTIES
            IMPORTED_LOCATION "${NPP_NPPC_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${NPP_INCLUDE_DIR}")

        add_library(NPP::nppial SHARED IMPORTED)
        set_target_properties(NPP::nppial PROPERTIES
            IMPORTED_LOCATION "${NPP_NPPIAL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${NPP_INCLUDE_DIR}")

        add_library(NPP::nppisu SHARED IMPORTED)
        set_target_properties(NPP::nppisu PROPERTIES
            IMPORTED_LOCATION "${NPP_NPPISU_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${NPP_INCLUDE_DIR}")
    endif()
endif()

mark_as_advanced(
    NPP_INCLUDE_DIR
    NPP_NPPC_LIBRARY
    NPP_NPPIAL_LIBRARY
    NPP_NPPISU_LIBRARY
    NPP_NPPIG_LIBRARY
    NPP_NPPIDEI_LIBRARY
    NPP_NPPICC_LIBRARY
    NPP_NPPIF_LIBRARY
    NPP_NPPS_LIBRARY)
