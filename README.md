# 27-NEXT-E-RADAR-VISION

XAUT NEXT-E 雷达视觉项目，包含海康工业相机采集、图像去畸变，以及基于 YOLO11、CUDA、NPP 和 TensorRT 的识别模块开发。

当前主要在 Linux 环境开发。相机模块提供独立示例；C++ 识别器仍在开发 GPU 图像预处理，尚未接通完整的模型推理和后处理流程。

## 目录结构

```text
.
├── CMakeLists.txt                 # 统一构建入口
├── camera_capture/
│   ├── include/                  # 相机与去畸变接口
│   ├── src/                      # 实现及运行示例
│   └── config/                   # 相机标定参数
├── detector_yolo11/
│   ├── cmake/                    # TensorRT、NPP 查找脚本
│   ├── include/                  # 识别器接口
│   ├── src/                      # CUDA 预处理、训练及模型转换脚本
│   ├── config/                   # 数据集配置
│   ├── model/                    # PT、ONNX、TensorRT 模型
│   ├── docs/                     # 环境搭建记录
│   └── run.sh                    # 训练脚本入口
└── ultralytics/                  # 仓库内的 Ultralytics 源码
```

## 模块与构建目标

| 目标 | 类型 | 用途 |
| --- | --- | --- |
| `hik_camera` | 动态库 | 海康相机采集封装 |
| `camera_calibration` | 动态库 | 读取已有标定参数并校正图像畸变 |
| `hik_camera_test` | 可执行文件 | 相机取流及图像显示 |
| `camera_calibration_test` | 可执行文件 | 原图与去畸变图像显示 |
| `preprocess_gpu` | 静态库 | 识别器 GPU 预处理，开发中 |

`cuda_env`、`tensorrt_env`、`opencv_env` 和 `npp_env` 是 CMake 接口目标，只传递依赖配置，不生成独立库文件。

## 环境依赖

- CMake 3.22.1 或以上，以及 C++ 编译器。
- 海康 MVS SDK。当前相机配置使用 `/opt/MVS/include` 和 `/opt/MVS/lib/64/libMvCameraControl.so`。
- OpenCV C++ 开发库。识别器使用 `cv::cuda::GpuMat`，运行 GPU 预处理需要支持 CUDA 的 OpenCV 构建。
- NVIDIA GPU、驱动、CUDA Toolkit（包含 `nvcc`）、NPP 和 TensorRT C++ SDK。
- 训练与转换使用 Python 环境；转换脚本依赖 `onnx`、`onnxruntime` 和 `tensorrt`。

识别器当前将 `CMAKE_CUDA_ARCHITECTURES` 设置为 `120`，配置面向 RTX 5060。更换 GPU 时需要检查架构配置，使用能够编译对应架构的 CUDA Toolkit。

详细环境记录见 [YOLO11 / TensorRT 环境搭建](detector_yolo11/docs/yolo11_tensorrt_setup.md)。其中的绝对路径及环境版本对应开发机器，迁移时需按实际安装位置调整。

## 构建

在仓库根目录配置整个项目。以下示例沿用开发机器的 `yolo11` Conda 环境，要求其中已安装 CUDA 编译器及相关 SDK：

```bash
conda activate yolo11
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CUDA_COMPILER="$CONDA_PREFIX/bin/nvcc" \
  -DYOLO11_ENV="$CONDA_PREFIX"
cmake --build build -j
```

`YOLO11_ENV` 用于查找环境下的 `tensorrt-cpp/` 和 `targets/x86_64-linux/`。自定义安装目录可通过 `TensorRT_ROOT`、`NPP_ROOT` 指定；OpenCV 可通过 `OpenCV_DIR` 指向其 CMake 配置目录。

只需要相机模块时，可以单独构建，不必配置识别器的 CUDA、TensorRT 和 NPP 依赖：

```bash
cmake -S camera_capture -B camera_capture/cmake-build-debug
cmake --build camera_capture/cmake-build-debug -j
```

根项目会导出 `build/compile_commands.json`，根目录 `.clangd` 指向这个构建目录。使用其他构建目录时，需要让编辑器读取对应的编译数据库。

## 相机示例

连接相机，确认 MVS SDK 可以访问设备，并在具有图形显示环境的终端运行：

```bash
./build/camera_capture/hik_camera_test
./build/camera_capture/camera_calibration_test
```

单独构建相机模块时，可执行文件位于 `camera_capture/cmake-build-debug/`。示例使用索引为 `0` 的相机，按 `q` 或 `Esc` 退出。

去畸变示例目前在源码中写死了 `camera0_calibration.xml` 的绝对路径。换机器前需调整路径，并使用对应相机的标定参数。该示例读取已有参数进行去畸变，不是采集棋盘格并求解标定参数的程序。

## 模型转换与训练

### ONNX 转 TensorRT

激活配置好依赖的 Python 环境后，在仓库根目录运行：

```bash
python detector_yolo11/src/onnx_to_trt.py \
  detector_yolo11/model/onnx/armor_best_640.onnx \
  detector_yolo11/model/trt/armor_best_640.trt
```

当前转换脚本按 TensorRT 11 API 编写，默认进行 FP16 转换，使用模型的静态输入形状。输出包括原始 TensorRT 引擎 `.trt`；如果 ONNX 带有 Ultralytics 元数据，还会生成带元数据头的 `.engine`。自定义 C++ 反序列化入口应使用原始引擎，不能将带元数据头的整个文件直接当作引擎数据。

### 训练

训练脚本为 `detector_yolo11/src/train.py`。当前配置使用 `yolo11n.yaml`、`coco8.yaml`、400 个 epoch、640 输入尺寸和 `device=[0,1]`，并在训练前调用 `model.val()`。这些是当前代码设置，实际训练前需按数据集、模型和可用 GPU 检查配置。

`run.sh` 使用 `$HOME/anaconda3` 下的 Conda 和 `yolo11` 环境，并通过当前工作目录定位脚本，因此需从识别器目录运行：

```bash
cd detector_yolo11
bash run.sh
```

## 当前开发状态

- 已有相机采集与去畸变实现及示例。
- 已有 ONNX 到 TensorRT 的转换脚本和模型文件。
- C++ 识别器已有图像上传、NPP Bayer 转 RGB 和缩放代码，预处理流程仍需完善和验证。
- 当前识别器没有独立可执行入口，完整 TensorRT 推理及后处理尚未接入。
- NPP 查找脚本会查找部分额外组件，但当前 `npp_env` 仅链接 `nppc`、`nppial`、`nppisu`。接入预处理调用的最终可执行程序时，还需核对颜色转换、缩放函数所需的组件链接。

## 代码规范与许可证

本项目遵循 [Google C++ 风格指南](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/index.html)。

项目根目录许可证见 [LICENSE](LICENSE)（GPL v3）。仓库内第三方代码及 SDK 的使用还需遵循各自许可证。
