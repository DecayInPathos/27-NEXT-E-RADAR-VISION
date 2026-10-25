# YOLO11 + TensorRT 环境配置、验证与 CMake 集成

> 机器：Ubuntu 22.04.5，RTX 5060 Laptop（Blackwell，SM 12.0，8 GB），驱动 595.99.02（CUDA 13.2）
> 环境：conda 环境 `yolo11`，Python 3.12.15
> 日期：2026-10-20
> 状态：环境已安装并完成 PyTorch / YOLO11 / TensorRT / C++ CMake 验证

本文记录**下载地址、复现命令、验证结果、CMake 与头文件引入方式**。按你的要求，没有改动仓库里的任何代码；只新增了本文档。

---

## 0. 快速使用

```bash
conda activate yolo11

# 检查
python -c "import torch, tensorrt, ultralytics; print(torch.__version__, torch.cuda.is_available(), tensorrt.__version__, ultralytics.__version__)"

# Python 推理（使用 GPU）
python -c "from ultralytics import YOLO; YOLO('/home/pathos/27-NEXT-E-RADAR-VISION/detector/model/armor_best.pt').predict('/tmp/yolo11_verify/bus.jpg', imgsz=640, device=0)"
```

C++ 编译（使用本文第 6 节的 CMake 配置）：

```bash
conda activate yolo11
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/<你的目标>
```

---

## 1. 已安装环境总览

| 组件 | 版本 | 位置/说明 |
|---|---|---|
| Python | 3.12.15 | `/home/pathos/anaconda3/envs/yolo11` |
| PyTorch | 2.14.1+cu130 | cuDNN 9.24.0.43，支持 SM 12.0 |
| torchvision | 0.29.1 | pip |
| Ultralytics | 8.4.183 | YOLO11 训练/推理/导出 |
| TensorRT（Python） | 11.4.0.106 | `tensorrt_cu13` + libs + bindings |
| ONNX | 1.23.2 | pip |
| onnxslim | 0.1.98 | ONNX 化简 |
| onnxruntime-gpu | 1.31.0 | Ultralytics 导出时自动安装；非 TensorRT 必需 |
| numpy | 2.5.4 | pip |
| opencv-python | 5.0.0.93 | pip |
| CUDA（conda，C++ 头文件/运行库） | cudart/crt 13.2.86 | `$CONDA_PREFIX/targets/x86_64-linux` |
| CUDA nvcc | 13.2.86 | conda `cuda-nvcc`，`$CONDA_PREFIX/bin/nvcc` |
| TensorRT C++ SDK（本地整理） | 11.4.0.106 | `$CONDA_PREFIX/tensorrt-cpp` |
| NVIDIA NPP | 13.1.0.59 | conda `libnpp` / `libnpp-dev`，CUDA 13.2 构建 |
| 编译器 / CMake | g++ 11.4.0 / CMake 3.22.1 | 系统 |

GPU 实测：

```text
NVIDIA GeForce RTX 5060 Laptop GPU
compute capability = 12.0 (sm_120)
VRAM = 7.50 GiB 可见 / 8 GB 物理显存
FP16 matmul ≈ 34.6 TFLOPS, BF16 matmul ≈ 35.9 TFLOPS
```

PyTorch 使用的 CUDA 运行时是 **13.0**（pip wheel），C++ 侧安装的 conda CUDA 头文件/运行库是 **13.2.86**。TensorRT 是 CUDA 13 构建；CUDA 13.x 小版本 ABI 兼容，实测 Python 与 C++ 均正常。

环境占用约 **11 GB**（其中 TensorRT pip 库约 4.4 GB）。

---

## 2. 下载地址

### 2.1 NVIDIA 驱动 / CUDA

- 驱动下载：<https://www.nvidia.com/download/index.aspx>
  - 本机已安装 **595.99.02**，`nvidia-smi` 显示支持 CUDA 13.2，无需重装。
- CUDA Toolkit 下载：<https://developer.nvidia.com/cuda-downloads>
- CUDA 历史版本：<https://developer.nvidia.com/cuda-toolkit-archive>
- cuDNN 下载（PyTorch 已自带 cuDNN 9.24，不需要单独安装）：<https://developer.nvidia.com/cudnn>

> RTX 50 系列（SM 12.0）必须使用 CUDA 12.8 及以上的 PyTorch 构建；本环境使用 cu130。

### 2.2 TensorRT 11.4（本环境使用）

官方页面：

- TensorRT 11.x 下载页：<https://developer.nvidia.com/tensorrt/download/11x>
- TensorRT 安装文档：<https://docs.nvidia.com/deeplearning/tensorrt/latest/installing-tensorrt/installing.html>
- TensorRT Release Notes：<https://docs.nvidia.com/deeplearning/tensorrt/release-notes/>
- TensorRT GitHub（头文件来源）：<https://github.com/NVIDIA/TensorRT>
  - 本环境对应 tag：<https://github.com/NVIDIA/TensorRT/tree/v11.4/include>
  - 头文件包：<https://github.com/NVIDIA/TensorRT/archive/refs/tags/v11.4.tar.gz>
- NVIDIA PyPI 真实 wheel 索引：<https://pypi.nvidia.com/>
- PyPI 上的 TensorRT 引导包：<https://pypi.org/project/tensorrt/>

官方 tar 包（可选，含 C++ 头文件、库、`trtexec`，适合需要完整 C++ SDK 的场景）：

```text
https://developer.nvidia.com/downloads/compute/machine-learning/tensorrt/11.4.0/tars/
TensorRT-Enterprise-11.4.0.106-Linux-x86_64-cuda-13.4-Release-external.tar.zst
```

该文件约 **3.73 GB**（已验证 URL 可直接下载，302 跳转到 `developer.download.nvidia.com`）。本环境没有下载它，而是用 pip 库 + GitHub 头文件组了 C++ SDK；两者的 TensorRT 版本号一致（11.4.0.106）。

#### Python TensorRT 的真实 wheel（重点）

PyPI 上的 `tensorrt` / `tensorrt-cu13` / `tensorrt-cu13-libs` 是**引导包**：其中 `tensorrt_cu13_libs` 的 sdist 只有约 15 KB，安装时会去 `pypi.nvidia.com` 下载约 **3.81 GB** 的真实 wheel 并校验 SHA256。网络不稳定时会出现：

```text
AssertionError: Downloaded wheel and sha256 don't match!
```

本环境第一次安装就遇到这个问题。解决办法是**直接从 NVIDIA 索引下载真实 wheel**，放到本地目录后用 `--find-links` 安装：

```bash
mkdir -p ~/trt_wheels && cd ~/trt_wheels

# 3.81 GB
curl -L --retry 20 --retry-delay 3 --retry-all-errors -C - -O \
  https://pypi.nvidia.com/tensorrt-cu13-libs/tensorrt_cu13_libs-11.4.0.106-py3-none-manylinux_2_28_x86_64.whl

# 1.3 MB（注意 Python 版本；Python 3.12 用 cp312）
curl -L -O \
  https://pypi.nvidia.com/tensorrt-cu13-bindings/tensorrt_cu13_bindings-11.4.0.106-cp312-none-manylinux_2_28_x86_64.whl

# TensorRT 的两个小引导 sdist（来自 PyPI）
curl -L -O \
  https://files.pythonhosted.org/packages/e2/ec/47f25e4ba319ba56d89dfc63488a777545a3001f2918472e58513b1eccd3/tensorrt-11.4.0.106.tar.gz
curl -L -O \
  https://files.pythonhosted.org/packages/57/df/a0044af5a9becb484004fcbfb9bf91758ae7bc42dc85a5a91dd30d0c420a/tensorrt_cu13-11.4.0.106.tar.gz

sha256sum *.whl
```

本次下载的校验值：

```text
tensorrt_cu13_libs-11.4.0.106-py3-none-manylinux_2_28_x86_64.whl
  c538a1d6203e2f5de0604ddadcf4e1df4099c645e0b58201754fb281f5d562de

tensorrt_cu13_bindings-11.4.0.106-cp312-none-manylinux_2_28_x86_64.whl
  5fc7a6c412881f7212030893bd5531b1a0ff415c72c7678dfe4ead7a7765b38d
```

安装：

```bash
conda activate yolo11
cd ~/trt_wheels
pip install --no-index --find-links . --no-build-isolation "tensorrt==11.4.0.106"
python -c "import tensorrt as trt; print(trt.__version__)"
# 11.4.0.106
```

> `--no-build-isolation` 是必须的：`--no-index` 下无法联网安装构建依赖，而环境里已有 `setuptools 84.0.0`。
>
> wheel 里只有 `libnvinfer.so.11`、`libnvinfer_plugin.so.11`、`libnvonnxparser.so.11` 和 builder resources，**没有 C++ 头文件，也没有 `libnvinfer.so` 这种开发用软链接**。C++ 部分见第 6 节。

### 2.3 PyTorch

- 官方安装页：<https://pytorch.org/get-started/locally/>
- cu130 wheel 索引：<https://download.pytorch.org/whl/cu130>
- 本次使用的镜像：<https://pypi.tuna.tsinghua.edu.cn/simple>

```bash
conda create -y -n yolo11 python=3.12
conda activate yolo11
pip install torch==2.14.1 torchvision==0.29.1 \
  -i https://pypi.tuna.tsinghua.edu.cn/simple

# 或者官方 cu130 索引
# pip install torch==2.14.1 torchvision==0.29.1 \
#   --index-url https://download.pytorch.org/whl/cu130
```

安装后应看到：

```text
torch 2.14.1+cu130
torch.version.cuda == 13.0
torch.cuda.get_device_capability(0) == (12, 0)
```

> 如果使用 cu128 及以下的 wheel，RTX 50 系列可能报 “no kernel image is available”。本机已验证 cu130 正常。

### 2.4 Ultralytics / YOLO11

- YOLO11 文档：<https://docs.ultralytics.com/models/yolo11/>
- TensorRT 集成文档：<https://docs.ultralytics.com/integrations/tensorrt/>
- GitHub：<https://github.com/ultralytics/ultralytics>
- PyPI：<https://pypi.org/project/ultralytics/>
- 官方权重：<https://github.com/ultralytics/assets/releases>

```bash
pip install ultralytics==8.4.183 \
  -i https://pypi.tuna.tsinghua.edu.cn/simple
```

### 2.5 ONNX 工具

- ONNX：<https://github.com/onnx/onnx> / <https://pypi.org/project/onnx/>
- onnxslim：<https://github.com/inisis/onnxslim> / <https://pypi.org/project/onnxslim/>
- onnxruntime-gpu：<https://pypi.org/project/onnxruntime-gpu/>
- ONNX Runtime FP16 转换说明：<https://onnxruntime.ai/docs/performance/model-optimizations/float16.html>

```bash
pip install onnx==1.23.2 onnxslim -i https://pypi.tuna.tsinghua.edu.cn/simple
```

`onnxruntime-gpu` 是 Ultralytics 在第一次导出时自动安装的（1.31.0），不用于 TensorRT 推理本身。

### 2.6 本项目模型

模型位于：

```text
/home/pathos/27-NEXT-E-RADAR-VISION/detector/model/
```

这些模型是 Ultralytics **8.3.220** 训练/导出的（ONNX 由 PyTorch 2.8.0 导出），在 8.4.183 下加载和推理已验证正常。

| 文件 | 任务 | imgsz | 输入 | 输出 | 类别 |
|---|---|---|---|---|---|
| `armor_best.pt/.onnx/.engine` | detect | 640 | `[1,3,640,640]` float32 | `[1,7,8400]` | 3：armor_red / armor_blue / armor_other |
| `arrmor_192_best.pt/.onnx/.engine` | detect | 192 | `[1,3,192,192]` float32 | `[1,7,756]` | 同上 3 类 |
| `car_1280_best.pt/.onnx/.engine` | detect | 1280 | `[1,3,1280,1280]` float32 | `[1,9,33600]` | 5：car / armor / ignore / watcher / base |
| `cls_best.pt/.onnx/.engine` | classify | 224 | `[1,3,224,224]` float32 | `[1,14]` | 14：B0–B5, BS, R0–R5, RS |
| `cls_64_best.pt/.onnx/.engine` | classify | 64 | `[1,3,64,64]` float32 | `[1,14]` | 同上 14 类 |

输出是 YOLO11 的**原始输出**（bbox + class scores），engine 里没有 NMS；C++ 部署需要自己做后处理（置信度阈值 + NMS）。检测类输出的布局是 `[1, 4+nc, anchors]`，其中前 4 个通道为 `cx, cy, w, h`。

### 2.7 pip 镜像

本次使用清华源：

```text
https://pypi.tuna.tsinghua.edu.cn/simple
```

NVIDIA 真实 wheel **不在**清华 PyPI 镜像里；必须用 `https://pypi.nvidia.com/` 或 NVIDIA 官方 tar。

### 2.8 NVIDIA NPP（图像预处理）

NPP 是 CUDA 的 2D 图像/信号处理库，常用于 GPU 上的缩放、颜色转换、滤波等预处理。

- 官方页面：<https://developer.nvidia.com/npp>
- NPP 随 CUDA Toolkit 提供，也可通过 conda 安装
- NPP+（C++ 高级接口）单独下载：<https://developer.nvidia.com/nppplus-downloads>
- 文档：<https://docs.nvidia.com/cuda/npp/>

本环境安装的是与 CUDA 13.2 匹配的版本：

```bash
conda install -y -n yolo11 -c nvidia "libnpp=13.1.0.59" "libnpp-dev=13.1.0.59"
```

说明：

- conda 包名是 **`libnpp` / `libnpp-dev`**，不是 `cuda-npp-dev`。
- NPP **13.2.x** 的 conda 包依赖 `cuda-version >=13.4`，会把当前 CUDA 13.2 栈升级；因此本环境使用 **13.1.0.59**（依赖 `cuda-version >=13.2`）。
- NPP 13.1 起不再提供 `libnppi.so` 总库，图像函数按模块拆到 `libnppial`、`libnppicc`、`libnppidei`、`libnppif`、`libnppig`、`libnppim`、`libnppist`、`libnppitc` 等，链接时按需选择。
- 头文件和库位置与 CUDA 相同：

```text
$CONDA_PREFIX/targets/x86_64-linux/include/npp.h
$CONDA_PREFIX/targets/x86_64-linux/include/nppi*.h
$CONDA_PREFIX/targets/x86_64-linux/lib/libnppc.so
$CONDA_PREFIX/targets/x86_64-linux/lib/libnppig.so
$CONDA_PREFIX/targets/x86_64-linux/lib/libnppidei.so
...
```

---

## 3. 复现步骤（从零到可用）

### 3.1 基础 Python 环境

```bash
conda create -y -n yolo11 python=3.12
conda activate yolo11
python -m pip install --upgrade pip

pip install torch==2.14.1 torchvision==0.29.1 ultralytics==8.4.183 \
  onnx==1.23.2 onnxslim -i https://pypi.tuna.tsinghua.edu.cn/simple
```

### 3.2 TensorRT Python

按第 2.2 节下载真实 wheel 后：

```bash
pip install --no-index --find-links ~/trt_wheels --no-build-isolation \
  "tensorrt==11.4.0.106"
python -c "import tensorrt as trt; print(trt.__version__); print(hasattr(trt, 'OnnxParser'))"
```

### 3.3 CUDA 头文件 / 运行库（C++ 用）

pip 的 CUDA 头文件不完整（缺 `crt/host_defines.h`），且 conda 的 13.0 `cuda-cudart-dev` 也未带 `crt/`。本环境安装的是 13.2（驱动支持，ABI 与 torch 的 13.0 兼容）：

```bash
conda install -y -n yolo11 -c nvidia "cuda-crt=13.2.86" "cuda-cudart-dev=13.2.86"
```

得到：

```text
$CONDA_PREFIX/targets/x86_64-linux/include/cuda_runtime_api.h
$CONDA_PREFIX/targets/x86_64-linux/include/crt/host_defines.h
$CONDA_PREFIX/targets/x86_64-linux/lib/libcudart.so
```

> 已额外安装 `cuda-nvcc=13.2.86`（`$CONDA_PREFIX/bin/nvcc`）。CMake 4.x 的 `find_package(CUDAToolkit)` 需要 nvcc 来定位 Toolkit；TensorRT C++ 推理本身不要求编译 `.cu`。如果以后写 CUDA 核，可直接使用这个 nvcc。

### 3.4 整理 TensorRT C++ SDK

pip 的 TensorRT wheel 不含 C++ 头文件。本环境用 GitHub tag `v11.4` 的头文件（版本号与 pip 库一致，`NvInferVersion.h` 中 `TRT_BUILD_ENTERPRISE = 106`），并从 pip 库创建开发用软链接：

```bash
SP="$CONDA_PREFIX/lib/python3.12/site-packages"
CPP="$CONDA_PREFIX/tensorrt-cpp"
mkdir -p "$CPP/include" "$CPP/lib"

curl -L -o /tmp/TensorRT-v11.4.tar.gz \
  https://github.com/NVIDIA/TensorRT/archive/refs/tags/v11.4.tar.gz
tar xzf /tmp/TensorRT-v11.4.tar.gz -C /tmp
cp -r /tmp/TensorRT-11.4/include/. "$CPP/include/"

for f in "$SP"/tensorrt_libs/*.so*; do
  ln -sf "$f" "$CPP/lib/$(basename "$f")"
done
ln -sf libnvinfer.so.11        "$CPP/lib/libnvinfer.so"
ln -sf libnvinfer_plugin.so.11 "$CPP/lib/libnvinfer_plugin.so"
ln -sf libnvonnxparser.so.11   "$CPP/lib/libnvonnxparser.so"
```

最终目录：

```text
$CONDA_PREFIX/tensorrt-cpp/
├── include/   NvInfer.h, NvInferRuntime.h, NvInferPlugin.h, NvOnnxParser.h, ...
└── lib/       libnvinfer.so -> libnvinfer.so.11, libnvinfer_plugin.so,
               libnvonnxparser.so, libnvinfer_builder_resource_sm120.so.11.4.0, ...
```

`libcudart` 不放进这里，CMake 直接链接 conda 的 CUDA 运行库。

### 3.5 快速验证

```bash
conda activate yolo11
python - <<'PY'
import torch, tensorrt as trt, ultralytics
print("torch:", torch.__version__, "| cuda:", torch.version.cuda, "| avail:", torch.cuda.is_available())
print("gpu:", torch.cuda.get_device_name(0), "| capability:", torch.cuda.get_device_capability(0))
print("tensorrt:", trt.__version__)
print("ultralytics:", ultralytics.__version__)
PY
```

### 3.6 NVIDIA NPP

```bash
conda activate yolo11
conda install -y -n yolo11 -c nvidia "libnpp=13.1.0.59" "libnpp-dev=13.1.0.59"

# 确认文件
ls "$CONDA_PREFIX/targets/x86_64-linux/include/nppi.h"
ls "$CONDA_PREFIX/targets/x86_64-linux/lib/libnppc.so" \
   "$CONDA_PREFIX/targets/x86_64-linux/lib/libnppig.so" \
   "$CONDA_PREFIX/targets/x86_64-linux/lib/libnppidei.so"
```

NPP 的编译/运行验证见 4.7，CMake 引入方式见 6.9。

---

## 4. 本次验证结果

所有验证都在 `/tmp/yolo11_verify` 下进行；**没有覆盖或修改项目里的 `.pt/.onnx/.engine`**。分类模型验证时 Ultralytics 曾把 ONNX 写回项目目录，已从 git 索引恢复原文件。

### 4.1 GPU / PyTorch

```text
torch 2.14.1+cu130
cuda_available = True
GPU: NVIDIA GeForce RTX 5060 Laptop GPU, sm_120
cuDNN: 9.24.0.43
FP16 matmul: 34.58 TFLOPS
BF16 matmul: 35.86 TFLOPS
FP16 Conv2d: OK
```

### 4.2 YOLO11 推理 / 训练

| 模型 | 结果 |
|---|---|
| `armor_best.pt` @640 | 推理 4.8 ms；端到端 **9.55 ms（≈105 FPS）**；在 bus.jpg 上 0 个检测（正常，模型只认装甲板） |
| `cls_best.pt` @224 | 推理 2.17 ms；端到端 **10.32 ms（≈97 FPS）**；top5 正常输出 |
| 官方 `yolo11n.pt` @640 | bus.jpg 检出 **bus + 4 person**，置信度 0.940/0.888/0.878/0.856/0.622；端到端 **9.94 ms（≈101 FPS）** |
| 训练 smoke test | 合成小数据集 1 epoch + AMP + 反向传播 + val 均通过，GPU 显存 0.16 GB |

说明：表中的“推理”是 Ultralytics 的纯网络时间；“端到端”包含 Python 预处理/后处理，适合和部署前的整体吞吐对比。

### 4.3 TensorRT 检测引擎

使用项目里的 `armor_best.pt` 复制到 `/tmp` 后导出（不覆盖原文件）。

| 版本 | 构建时间 | engine 大小 | 纯推理 | 端到端 |
|---|---:|---:|---:|---:|
| TensorRT FP32 @640 | 17.8 s | 11.0 MB | — | 6.46 ms |
| TensorRT **FP16** @640 | 20.5 s | 7.2 MB | **0.72 ms** | **6.06 ms（165 FPS）** |
| PyTorch FP32（参考） | — | — | 4.8 ms | 9.55 ms |

FP16 engine 的检测结果与 FP32 engine 对比（yolo11n，bus.jpg）：

```text
                  FP32 engine                     FP16 engine
bus        0.939  [11.9, 228.5, 799.3, 735.2]   0.939  [11.8, 228.2, 799.0, 735.3]
person     0.902  [48.6, 398.0, 243.2, 904.5]   0.902  [48.7, 398.0, 243.3, 904.7]
person     0.849  [670.5, 392.7, 810.0, 879.6]  0.849  [671.2, 392.3, 810.0, 879.2]
person     0.833  [223.1, 405.6, 345.2, 859.7]  0.832  [223.2, 405.6, 345.1, 860.0]
person     0.400  [0.0, 550.2, 66.0, 871.8]     0.399  [0.0, 550.1, 66.0, 871.6]
```

结论：FP16 与 FP32 的分类、数量完全一致，置信度差 ≤ 0.001，框坐标差约 1 px 以内。yolo11n FP16 的端到端速度为 **5.92 ms**。

### 4.4 TensorRT 分类引擎

`cls_64_best.pt` @64，FP16 engine：

| 版本 | 大小 | top5 | 端到端 |
|---|---:|---|---:|
| PyTorch | — | R0 0.9301, R1 0.0192, R4 0.0191, RS 0.0120, B1 0.0114 | 9.25 ms |
| TensorRT FP16 | 3.7 MB | R0 0.9302, R1 0.0192, R4 0.0189, RS 0.0119, B1 0.0114 | **6.74 ms（148 FPS）** |

分类结果与 PyTorch 一致。

### 4.5 C++ / CMake

本次实际编译运行的输出：

```text
TensorRT 11.4.0.106
GPU: NVIDIA GeForce RTX 5060 Laptop GPU sm_120
Builder / NetworkV2 / ONNX parser OK
```

另外用 `trt_infer` 实际加载第 4.3 节的 FP16 engine 并执行了一次推理，验证了 metadata 头跳过、反序列化、I/O 地址设置和 `enqueueV3`：

```text
skipped Ultralytics metadata header (552 bytes)
serialized engine: 7151100 bytes
input  images [1,3,640,640] dtype=0        # 0 = kFLOAT (FP32)
output output0 [1,7,8400]   dtype=0
enqueueV3=ok sync=no error out[0..4]=33.1562 37.125 45.5938 51.9062 52
C++ TensorRT inference OK
```

说明 C++ 头文件、库、CUDA 运行库、CMake 导入和运行时加载 engine 都可用。

### 4.6 旧 `.engine` 的兼容性（重要）

`model/` 下原有的 `.engine` 文件无法在本机 TensorRT 11.4 加载，四个都报：

```text
Serialization assertion stdVersionRead == kSERIALIZATION_VERSION failed.
Version tag does not match. Current Version: 245, Serialized Engine Version: 240
```

原因：这些 engine 是 **TensorRT 10.x** 序列化的（版本标签 240），当前运行时是 **TensorRT 11.4**（245）。TensorRT engine 与 TensorRT 版本、GPU 架构绑定，**不能跨大版本复用**。

处理方式：在**本机、当前环境**重新导出/构建 engine。原文件没有被覆盖；新 engine 可以放到单独目录（例如 `model/trt11/`），确认无误后再决定是否替换。

> 如果必须复用旧 engine，需要安装与之匹配的 TensorRT 10.x 运行时，并且最好在同一 GPU 架构上使用；本文不推荐这种做法。

### 4.7 NVIDIA NPP

CMake 构建的 `npp_smoke` 实际执行了一次 GPU 图像操作（4×4 RGB → 8×8，linear resize）：

```text
NPP 13.1.0
nppiSet_8u_C3R_Ctx=0 nppiResize_8u_C3R_Ctx=0 sync=no error
dst[0..5]=128 128 128 128 128 128 | all 128: yes
NPP smoke OK
```

动态链接确认：

```text
libnppig.so.13   -> $CONDA_PREFIX/targets/x86_64-linux/lib/libnppig.so.13
libnppidei.so.13 -> $CONDA_PREFIX/targets/x86_64-linux/lib/libnppidei.so.13
libnppc.so.13    -> $CONDA_PREFIX/targets/x86_64-linux/lib/libnppc.so.13
libcudart.so.13  -> $CONDA_PREFIX/targets/x86_64-linux/lib/libcudart.so.13
```

### 4.8 CLion / CMake 4.3

CLion 自带 CMake 4.3，**FindCUDA 已被移除**。项目已迁移到 `find_package(CUDAToolkit)` + 自定义 `FindTensorRT.cmake` / `FindNPP.cmake`。用 CLion 的 CMake 实测配置输出：

```text
Found CUDAToolkit: ... (found version "13.2.86")
Found TensorRT: /home/pathos/anaconda3/envs/yolo11/tensorrt-cpp/include
Found OpenCV: /usr (found version "4.5.4")
Found NPP: /home/pathos/anaconda3/envs/yolo11/targets/x86_64-linux/include
-- Configuring done
-- Generating done
```

真实编译并链接 `yolo_dependencies` 的测试程序输出：

```text
TensorRT 11.4.0.106
NPP 13.1.0
OpenCV 4.5.4
GPU NVIDIA GeForce RTX 5060 Laptop GPU sm_120
```

### 4.9 USE_GPU_PREPROCESS

构建系统已默认启用该宏。用一个链接 `yolo_dependencies` 的测试目标验证宏能正确传播：

```text
-- GPU preprocessing: ENABLED (USE_GPU_PREPROCESS)

USE_GPU_PREPROCESS=ON
TensorRT 11.4.0.106
NPP 13.1.0
OpenCV 4.5.4
GPU NVIDIA GeForce RTX 5060 Laptop GPU sm_120
```

CLion 的构建缓存中也确认：

```text
USE_GPU_PREPROCESS:BOOL=ON
```

> 目前 `src/` 还是空的，所以宏已定义但还没有业务代码使用它；添加源文件并链接 `yolo_dependencies` 后即可生效。

---

## 5. 重新生成 TensorRT engine

### 5.1 推荐流程（不覆盖原文件）

```bash
conda activate yolo11
mkdir -p /tmp/trt_export
cp model/armor_best.pt /tmp/trt_export/

cd /tmp/trt_export
python -c "from ultralytics import YOLO; YOLO('armor_best.pt').export(format='onnx', imgsz=640, quantize=None, device=0)"

# 用第 5.3 节的脚本构建 FP16 engine
python build_engine_fp16.py \
  --onnx armor_best.onnx \
  --out armor_best_fp16.engine \
  --metadata-from /home/pathos/27-NEXT-E-RADAR-VISION/detector/model/armor_best.engine
```

各模型对应 `imgsz`：640（armor）、192（armor_192）、1280（car）、224（cls）、64（cls_64）。

### 5.2 Ultralytics 自带导出（可选）

Ultralytics 8.4 中 `half=True` 已弃用，TensorRT 11 又移除了 FP16 builder flag。现在它要求使用 **NVIDIA ModelOpt** 把 FP16 写进 ONNX：

```python
model.export(format="engine", imgsz=640, quantize=16, device=0)
```

这条路需要 `nvidia-modelopt[onnx]`。本环境**没有安装 ModelOpt**，原因：

- Ultralytics 自动安装时会执行 `pip install nvidia-modelopt[onnx]>=0.44 torch==2.14.1+cu130`，而 PyPI 上没有 `torch==2.14.1+cu130` 这个本地版本号，自动安装必然失败；
- 手动安装会拉 `cupy-cuda12x`、`onnx~=1.21.0`，并可能降级现有 onnx/onnxruntime，影响本环境。

因此本环境使用第 5.3 节的 **ONNX Runtime FP16 转换 + TensorRT API** 方案。该方案不依赖 ModelOpt，已验证检测和分类都正常。

> 如果以后确实要用 ModelOpt，建议单独建一个环境，或先 `pip install "nvidia-modelopt[onnx]>=0.44"`（不要带 torch pin），再导出。

### 5.3 FP16 engine 构建脚本（核心）

`TensorRT 11` 是强类型网络：精度由 ONNX 图中的类型决定，不再有 `BuilderFlag.FP16`。下面脚本把 ONNX 的权重/中间值转成 FP16（**保持输入/输出为 FP32**），再构建 engine，并把 Ultralytics 的元数据头补回去。

```python
#!/usr/bin/env python3
# 用法:
#   python build_engine_fp16.py --onnx model.onnx --out model_fp16.engine \
#       --metadata-from reference.engine
import argparse, json, struct, time
from pathlib import Path

import onnx
import tensorrt as trt
from onnxruntime.transformers.float16 import convert_float_to_float16


def convert_fp16(onnx_path, fp16_path):
    m = onnx.load(str(onnx_path))
    m16 = convert_float_to_float16(m, keep_io_types=True)  # 输入/输出保持 FP32
    onnx.save(m16, str(fp16_path))


def build(onnx_path, out_path, workspace_gb=4.0):
    logger = trt.Logger(trt.Logger.WARNING)
    trt.init_libnvinfer_plugins(logger, "")
    builder = trt.Builder(logger)
    network = builder.create_network(0)          # TRT 11：显式 batch 是默认
    parser = trt.OnnxParser(network, logger)
    with open(onnx_path, "rb") as f:
        if not parser.parse(f.read()):
            for i in range(parser.num_errors):
                print(parser.get_error(i))
            raise RuntimeError("ONNX parse failed")
    cfg = builder.create_builder_config()
    cfg.set_memory_pool_limit(trt.MemoryPoolType.WORKSPACE, int(workspace_gb * (1 << 30)))
    cfg.builder_optimization_level = 5
    t0 = time.perf_counter()
    blob = builder.build_serialized_network(network, cfg)
    if blob is None:
        raise RuntimeError("TensorRT build failed")
    Path(out_path).write_bytes(blob)
    print(f"built {out_path} in {time.perf_counter() - t0:.1f}s, "
          f"{Path(out_path).stat().st_size / 1e6:.1f} MB")


def prepend_ultralytics_header(engine_path, ref_engine_path):
    """Ultralytics engine 文件格式: int32(LE) JSON长度 + JSON + 序列化 engine。"""
    ref = Path(ref_engine_path).read_bytes()
    n = struct.unpack("<i", ref[:4])[0]
    json.loads(ref[4:4 + n].decode())       # 校验确实是 Ultralytics 头
    raw = Path(engine_path).read_bytes()
    with open(engine_path, "wb") as f:
        f.write(struct.pack("<i", n))
        f.write(ref[4:4 + n])
        f.write(raw)
    print(f"prepended {n}-byte metadata header")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--onnx", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--metadata-from", help="参考 .engine（Ultralytics 导出的任意同模型 engine）")
    ap.add_argument("--workspace", type=float, default=4.0)
    a = ap.parse_args()

    fp16_onnx = Path(a.out).with_name(Path(a.out).stem + ".fp16.onnx")
    convert_fp16(a.onnx, fp16_onnx)
    build(fp16_onnx, a.out, a.workspace)
    if a.metadata_from:
        prepend_ultralytics_header(a.out, a.metadata_from)
```

说明：

- `keep_io_types=True` 让 engine 输入/输出保持 **float32**，C++ 里按 float 准备数据即可；内部卷积/激活是 FP16。
- `--metadata-from` 只是用来复制 Ultralytics 的 JSON 元数据（imgsz/nms/names/task）。旧 engine 虽然加载不了，但它的**文件头仍然可读**，可以拿来做参考。
- 如果不用 Ultralytics 的 `YOLO(engine)` 加载，而是 C++ 自己 deserialize，metadata 头可以不加；但如果加了，C++ 必须先跳过这个头（见第 6.6 节）。
- `create_network(0)` 是 TensorRT 11 的写法；如果改成 TensorRT 10.x，需要 `1 << int(trt.NetworkDefinitionCreationFlag.EXPLICIT_BATCH)`。

### 5.4 什么时候必须重建 engine

以下任一变化都需要重建：

- TensorRT 大版本/小版本变化（本次旧 engine 240 → 新运行时 245 就无法加载）；
- GPU 架构变化（sm_86 / sm_89 / sm_120 之间通常不通用）；
- 模型、输入尺寸、batch、精度变化；
- 部署到另一台机器。

### 5.5 TensorRT 8/10 脚本迁移到 11

`src/onnx_to_trt.py` 已按 TensorRT 11.4 更新。常见 API 变化：

| 旧写法（TRT 8/10） | TensorRT 11.4 |
|---|---|
| `NetworkDefinitionCreationFlag.EXPLICIT_BATCH` | **已删除**；显式 batch 是唯一模式，用 `builder.create_network()` |
| `builder.platform_has_fast_fp16` | **已删除** |
| `config.set_flag(trt.BuilderFlag.FP16)` | **已删除**；FP16 必须写进 ONNX 图 |
| `profile.set_shape("input", ...)` | 用 `network.get_input(0).name`；静态模型不需要 profile |
| `builder.build_engine(...)` | `builder.build_serialized_network(...)` |
| `builder.max_batch_size` | 已删除；batch 由 ONNX 输入形状决定 |

当前脚本行为：

- 默认将 `model/onnx/car_1280_best.onnx` 转为两份文件：
  - `model/trt/car_1280_best.trt`：**裸 engine**，给自定义 C++ `deserializeCudaEngine` 直接使用
  - `model/trt/car_1280_best.engine`：同一份 engine + **Ultralytics metadata 头**，给 `YOLO(engine)` 使用
- metadata 直接从 ONNX 的 `metadata_props` 自动生成（task、imgsz、batch、channels、names、args 等），不需要手工复制旧 engine 的头
- `fp16_mode=True` 时用 ONNX Runtime 的 `convert_float_to_float16(..., keep_io_types=True)` 生成 FP16 ONNX，再构建 engine；engine 的输入/输出仍然是 FP32
- 支持命令行指定其他模型：

```bash
python3 onnx_to_trt.py <input.onnx> <output.trt>
```

- 本次运行生成 **9.5 MB** 两份文件，I/O 为 `images [1,3,1280,1280] FP32` → `output0 [1,9,33600] FP32`
- 已验证 `YOLO(car_1280_best.engine)` 可以直接加载，`names` 正确为 `car/armor/ignore/watcher/base`，并能推理（约 2.75 ms）

> 使用区分：C++ 自定义代码用 `.trt`（第 0 字节就是 TensorRT engine）；Ultralytics 用 `.engine`（前 4 字节是 metadata 长度，C++ 读取时需要跳过）。原生 `yolo export format=engine` 也会生成带 metadata 的 engine，但当前 Ultralytics 8.4 的 FP16 导出需要 ModelOpt；本脚本保留 FP16 路线，同时自动补 metadata。

---

## 6. C++ / CMake 集成（已实测）

### 6.1 路径总览

```text
$CONDA_PREFIX = /home/pathos/anaconda3/envs/yolo11

$CONDA_PREFIX/tensorrt-cpp/include      # NvInfer.h, NvOnnxParser.h, ...
$CONDA_PREFIX/tensorrt-cpp/lib          # libnvinfer.so, libnvinfer_plugin.so, libnvonnxparser.so

$CONDA_PREFIX/targets/x86_64-linux/include   # cuda_runtime_api.h, crt/host_defines.h, ...
$CONDA_PREFIX/targets/x86_64-linux/lib       # libcudart.so
```

### 6.2 头文件包含

```cpp
#include <NvInfer.h>              // IBuilder / INetworkDefinition / IRuntime / ICudaEngine ...
#include <NvInferPlugin.h>        // initLibNvInferPlugins()
#include <NvOnnxParser.h>         // nvonnxparser::createParser()（只在 C++ 里构建 engine 时需要）
#include <cuda_runtime_api.h>     // cudaStream_t / cudaMalloc / cudaMemcpyAsync ...
```

要求 **C++17** 或更高。本机用 g++ 11.4.0 编译通过。

### 6.3 可直接使用的 CMakeLists.txt

下面这份是本次实际编译/运行验证过的配置（项目名和源文件按你的工程改）：

```cmake
cmake_minimum_required(VERSION 3.22)
project(detector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{CONDA_PREFIX})
    message(FATAL_ERROR "CONDA_PREFIX is not set. Run: conda activate yolo11")
endif()
set(ENV_PREFIX "$ENV{CONDA_PREFIX}")

set(TRT_ROOT  "${ENV_PREFIX}/tensorrt-cpp")
set(CUDA_ROOT "${ENV_PREFIX}/targets/x86_64-linux")

find_path(TRT_INCLUDE_DIR NvInfer.h
    PATHS "${TRT_ROOT}/include" NO_DEFAULT_PATH REQUIRED)
find_library(TRT_NVINFER_LIB nvinfer
    PATHS "${TRT_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(TRT_ONNXPARSER_LIB nvonnxparser
    PATHS "${TRT_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(TRT_PLUGIN_LIB nvinfer_plugin
    PATHS "${TRT_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)
find_path(CUDA_INCLUDE_DIR cuda_runtime_api.h
    PATHS "${CUDA_ROOT}/include" NO_DEFAULT_PATH REQUIRED)
find_library(CUDA_CUDART_LIB cudart
    PATHS "${CUDA_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)

add_library(TRT::nvinfer SHARED IMPORTED)
set_target_properties(TRT::nvinfer PROPERTIES
    IMPORTED_LOCATION "${TRT_NVINFER_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${TRT_INCLUDE_DIR};${CUDA_INCLUDE_DIR}")

add_library(TRT::nvonnxparser SHARED IMPORTED)
set_target_properties(TRT::nvonnxparser PROPERTIES
    IMPORTED_LOCATION "${TRT_ONNXPARSER_LIB}")

add_library(TRT::nvinfer_plugin SHARED IMPORTED)
set_target_properties(TRT::nvinfer_plugin PROPERTIES
    IMPORTED_LOCATION "${TRT_PLUGIN_LIB}")

add_library(CUDA::cudart SHARED IMPORTED)
set_target_properties(CUDA::cudart PROPERTIES
    IMPORTED_LOCATION "${CUDA_CUDART_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIR}")

add_executable(detector src/main.cpp)   # 按你的源文件改
target_link_libraries(detector PRIVATE
    TRT::nvinfer
    TRT::nvonnxparser
    TRT::nvinfer_plugin
    CUDA::cudart)

# 编译产物直接带 RPATH，运行时不用手动 export LD_LIBRARY_PATH
set_target_properties(detector PROPERTIES
    BUILD_RPATH   "${TRT_ROOT}/lib;${CUDA_ROOT}/lib"
    INSTALL_RPATH "${TRT_ROOT}/lib;${CUDA_ROOT}/lib")
```

注意：

- 当前仓库里的 `detector/CMakeLists.txt` 第一行是 `cmkae_minimum_required`（拼写错误），且没有 target；按你的要求我没有改它。上面的文件是独立可用的示例。
- `find_library(... nvinfer ...)` 能成功，是因为我们在 `tensorrt-cpp/lib` 里创建了 `libnvinfer.so -> libnvinfer.so.11` 等软链接。pip wheel 本身只有带版本号的 `.so`。
- 如果改用 NVIDIA 官方 TensorRT tar 包，把 `TRT_ROOT` 改成 `.../TensorRT-11.4.0.106`，其中已经包含 `include/`、`lib/`（以及 `bin/trtexec`）。

### 6.4 编译 / 运行

```bash
conda activate yolo11
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/detector
```

如果运行时提示找不到动态库（例如把二进制拷到别处、RPATH 丢失）：

```bash
export LD_LIBRARY_PATH="$CONDA_PREFIX/tensorrt-cpp/lib:$CONDA_PREFIX/targets/x86_64-linux/lib:$LD_LIBRARY_PATH"
```

Python 侧不需要设置 `LD_LIBRARY_PATH`，`import tensorrt` 会自动找到 pip 包里的库。

### 6.5 TensorRT 11 C++ 构建 engine 的 API 要点

```cpp
class Logger : public nvinfer1::ILogger {
    void log(Severity s, const char* msg) noexcept override { /* ... */ }
};

Logger logger;
initLibNvInferPlugins(&logger, "");
auto builder = std::unique_ptr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(logger));
auto network = std::unique_ptr<nvinfer1::INetworkDefinition>(builder->createNetworkV2(0));
auto parser  = std::unique_ptr<nvonnxparser::IParser>(
                   nvonnxparser::createParser(*network, logger));

const int verbosity = static_cast<int>(nvinfer1::ILogger::Severity::kWARNING);
parser->parseFromFile(onnx_path, verbosity);

auto config = std::unique_ptr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
config->setMemoryPoolLimit(nvinfer1::MemoryPoolType::kWORKSPACE, 4ULL << 30);
nvinfer1::IHostMemory* serialized = builder->buildSerializedNetwork(*network, *config);
// 保存 serialized->data() / serialized->size()
```

关键点：

- **TensorRT 11 没有 `BuilderFlag::kFP16`**。FP16 必须先写进 ONNX（ModelOpt 或 ONNX Runtime 的 float16 转换），否则构建出来就是 FP32。
- 不再需要设置 `maxBatchSize`；batch 由 ONNX 输入形状决定（本项目的模型都是 batch=1 静态形状）。
- engine 的 I/O 名字：检测模型 `images` → `output0`；分类模型也是 `images` → `output0`。
- 本项目 FP16 engine 的 I/O 仍然是 **FLOAT (FP32)**（`keep_io_types=True`），内部是 FP16。部署时输入按 float32 准备即可。

### 6.6 C++ 读取/反序列化 Ultralytics `.engine`（必须跳过头）

Ultralytics 导出的 `.engine` 不是裸 engine，而是：

```text
[4 字节小端 int32：JSON 长度 N][N 字节 JSON 元数据][TensorRT 序列化 engine]
```

C++ 直接 `deserializeCudaEngine(file, size)` 会失败。正确做法：

```cpp
#include <fstream>
#include <iterator>
#include <vector>

std::ifstream f(path, std::ios::binary);
int32_t n = 0;
f.read(reinterpret_cast<char*>(&n), sizeof(n));

bool has_meta = (n > 0 && n < 1024 * 1024);   // 粗略判断
if (has_meta) {
    f.seekg(n, std::ios::cur);                // 跳过 JSON
} else {
    f.seekg(0, std::ios::beg);                // 裸 engine
}
std::vector<char> blob((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());

nvinfer1::IRuntime* runtime = nvinfer1::createInferRuntime(logger);
nvinfer1::ICudaEngine* engine =
    runtime->deserializeCudaEngine(blob.data(), blob.size());
```

执行推理时（TRT 10/11 风格）：

```cpp
auto ctx = std::unique_ptr<nvinfer1::IExecutionContext>(engine->createExecutionContext());
ctx->setTensorAddress("images",  d_input);
ctx->setTensorAddress("output0", d_output);
ctx->enqueueV3(stream);                 // TensorRT 10/11 的异步执行接口
cudaStreamSynchronize(stream);
```

（更完整的实现可参考 TensorRT 官方 samples；本环境只验证了构建 engine 所需的 C++ 头文件/链接和 Builder/Parser 创建。）

### 6.7 可选：官方 TensorRT tar 与 `trtexec`

如果希望使用 NVIDIA 官方的完整 C++ SDK（包含 `include/`、`lib/`、`bin/trtexec`、Python wheels）：

```bash
curl -L -o /tmp/TensorRT-11.4.tar.zst \
  "https://developer.nvidia.com/downloads/compute/machine-learning/tensorrt/11.4.0/tars/TensorRT-Enterprise-11.4.0.106-Linux-x86_64-cuda-13.4-Release-external.tar.zst"

mkdir -p "$HOME/.local/tensorrt"
tar --zstd -xf /tmp/TensorRT-11.4.tar.zst -C "$HOME/.local/tensorrt"
ls "$HOME/.local/tensorrt/TensorRT-11.4.0.106"   # include lib bin python
```

然后把 CMake 的 `TRT_ROOT` 指向 `$HOME/.local/tensorrt/TensorRT-11.4.0.106`。下载/使用需接受 NVIDIA TensorRT License。

> 本环境当前的 pip + GitHub 头文件方案已通过编译验证；官方 tar 是可选的“大而全”方案，主要多出 `trtexec`。
>
> 注意：该官方 tar 标注为 **CUDA 13.4** 构建，而本机驱动当前显示支持到 CUDA 13.2。CUDA 13.x 小版本兼容通常可用，但若运行时报 CUDA runtime 版本不足，优先使用本文已验证的 pip cu13 方案，或升级驱动。Enterprise 包的使用还需遵守 NVIDIA 的许可条款。

### 6.8 关于 nvcc

已安装 `cuda-nvcc=13.2.86`。CMake 4.x 的 `FindCUDAToolkit` 需要它来定位 CUDA Toolkit：

```bash
$CONDA_PREFIX/bin/nvcc --version
# Cuda compilation tools, release 13.2, V13.2.86
```

TensorRT C++ 推理本身不需要编译 `.cu`，这个 nvcc 主要供 CMake 定位 Toolkit，以及以后写自定义 CUDA 核使用。若环境重建，可用：

```bash
conda install -y -n yolo11 -c nvidia "cuda-nvcc=13.2.86"
```

### 6.9 NVIDIA NPP 的 CMake 与头文件引入

NPP 的头文件在 CUDA include 目录里，和 `cuda_runtime_api.h` 同级；库在 CUDA lib 目录里。

头文件：

```cpp
#include <npp.h>       // 基本类型、NppStatus、NPP_NO_ERROR、版本查询
#include <nppi.h>      // 图像处理 API（内部包含 nppi_* 各模块头文件）
```

CMake（按实际用到的模块添加；下面是 resize + set 所需的最小集合）：

```cmake
# NPP 13.1 没有 libnppi.so 总库，需要按模块链接
find_library(NPPC_LIB nppc
    PATHS "${CUDA_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(NPPIG_LIB nppig
    PATHS "${CUDA_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(NPPIDEI_LIB nppidei
    PATHS "${CUDA_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)

add_library(NPP::nppc SHARED IMPORTED)
set_target_properties(NPP::nppc PROPERTIES
    IMPORTED_LOCATION "${NPPC_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIR}")

add_library(NPP::nppig SHARED IMPORTED)
set_target_properties(NPP::nppig PROPERTIES
    IMPORTED_LOCATION "${NPPIG_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIR}")

add_library(NPP::nppidei SHARED IMPORTED)
set_target_properties(NPP::nppidei PROPERTIES
    IMPORTED_LOCATION "${NPPIDEI_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIR}")

target_link_libraries(detector PRIVATE
    NPP::nppig
    NPP::nppidei
    NPP::nppc
    CUDA::cudart)
```

常用模块与用途：

| 库 | 用途 |
|---|---|
| `nppc` | 核心库，**任何 NPP 程序都要链接** |
| `nppig` | 几何变换：resize、warp、旋转 |
| `nppidei` | 数据交换/初始化：set、copy |
| `nppicc` | 颜色转换：RGB/BGR/YUV、色彩空间 |
| `nppif` | 滤波：box、gaussian、Sobel 等 |
| `nppial` | 算术/逻辑运算 |
| `nppim` | 形态学 |
| `nppist` | 统计 |
| `nppitc` | 阈值/比较 |

NPP 13.1 的 resize 等接口只有 `_Ctx` 版本，需要自己填 `NppStreamContext`（头文件的注释说明该结构由应用填充）：

```cpp
NppStreamContext make_npp_ctx(cudaStream_t stream) {
    NppStreamContext c{};
    int dev = 0;
    cudaDeviceProp p{};
    int major = 0;
    int minor = 0;
    unsigned int flags = 0;

    cudaGetDevice(&dev);
    cudaGetDeviceProperties(&p, dev);
    cudaDeviceGetAttribute(&major, cudaDevAttrComputeCapabilityMajor, dev);
    cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, dev);
    cudaStreamGetFlags(stream, &flags);

    c.hStream = stream;
    c.nCudaDeviceId = dev;
    c.nMultiProcessorCount = p.multiProcessorCount;
    c.nMaxThreadsPerMultiProcessor = p.maxThreadsPerMultiProcessor;
    c.nMaxThreadsPerBlock = p.maxThreadsPerBlock;
    c.nSharedMemPerBlock = p.sharedMemPerBlock;
    c.nCudaDevAttrComputeCapabilityMajor = major;
    c.nCudaDevAttrComputeCapabilityMinor = minor;
    c.nStreamFlags = flags;
    return c;
}
```

调用示例（4×4 RGB → 8×8）：

```cpp
NppStreamContext ctx = make_npp_ctx(stream);
NppiSize src_size{4, 4}, dst_size{8, 8};
NppiRect src_rect{0, 0, 4, 4}, dst_rect{0, 0, 8, 8};

NppStatus st = nppiResize_8u_C3R_Ctx(
    d_src, 4 * 3, src_size, src_rect,
    d_dst, 8 * 3, dst_size, dst_rect,
    NPPI_INTER_LINEAR, ctx);
```

运行库：

- 上述 NPP 库与 `libcudart.so` 都在 `$CONDA_PREFIX/targets/x86_64-linux/lib`。
- 第 6.3 节的 CMake 已经把 `${CUDA_ROOT}/lib` 放进 `BUILD_RPATH`，所以 NPP 不需要额外的 `LD_LIBRARY_PATH`。
- 如果手工用 g++ 编译：

```bash
P="$CONDA_PREFIX/targets/x86_64-linux"
g++ -std=c++17 -O2 npp_smoke.cpp \
  -I"$P/include" -L"$P/lib" \
  -lnppig -lnppidei -lnppc -lcudart \
  -Wl,-rpath,"$P/lib" -o npp_smoke
```

> NPP+ 是 NVIDIA 另一套 C++ 高级接口（单独下载），本环境安装的是标准 NPP，不是 NPP+。

### 6.10 当前项目的 CMakeLists（CMake 4.x / CLion）

CLion 自带 CMake 4.3，`find_package(CUDA)` 已不可用（`FindCUDA` 模块被删除）；`TensorRT` 和 `NPP` 也没有 CMake 自带的 Find 模块。当前项目已改成：

- `find_package(CUDAToolkit REQUIRED)` 替代 `find_package(CUDA)`
- 新增 `cmake/FindTensorRT.cmake`
- 新增 `cmake/FindNPP.cmake`
- `CUDA::cudart` 替代 `${CUDA_LIBRARIES}`
- `${NPP_LIBRARIES}` 替代裸写的 `nppc nppial nppisu`
- 显式补 include 目录和 RPATH

当前 `detector/CMakeLists.txt` 的关键部分：

```cmake
cmake_minimum_required(VERSION 3.22.1)
project(detector LANGUAGES CXX)

if(DEFINED ENV{CONDA_PREFIX} AND EXISTS "$ENV{CONDA_PREFIX}/tensorrt-cpp")
    set(_default_yolo11_env "$ENV{CONDA_PREFIX}")
else()
    set(_default_yolo11_env "/home/pathos/anaconda3/envs/yolo11")
endif()
set(YOLO11_ENV "${_default_yolo11_env}" CACHE PATH "Path to the yolo11 conda environment")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CUDAToolkit_ROOT)
    set(CUDAToolkit_ROOT "${YOLO11_ENV}" CACHE PATH "CUDA Toolkit root")
endif()
find_package(CUDAToolkit REQUIRED)
find_package(TensorRT REQUIRED)
find_package(OpenCV REQUIRED)
find_package(NPP REQUIRED)

option(USE_GPU_PREPROCESS "Enable GPU preprocessing (NPP/CUDA) in sources" ON)

add_library(yolo_dependencies INTERFACE)
target_include_directories(yolo_dependencies INTERFACE
        ${TensorRT_INCLUDE_DIRS}
        ${CUDAToolkit_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${NPP_INCLUDE_DIRS})
target_link_libraries(yolo_dependencies INTERFACE
        ${TensorRT_LIBRARIES}
        CUDA::cudart
        ${OpenCV_LIBS}
        ${NPP_LIBRARIES})
if(USE_GPU_PREPROCESS)
    target_compile_definitions(yolo_dependencies INTERFACE USE_GPU_PREPROCESS)
    foreach(_npp_component IN ITEMS
            NPP_NPPIG_LIBRARY NPP_NPPIDEI_LIBRARY NPP_NPPICC_LIBRARY)
        if(${_npp_component})
            target_link_libraries(yolo_dependencies INTERFACE "${${_npp_component}}")
        endif()
    endforeach()
endif()
target_link_options(yolo_dependencies INTERFACE
        "-Wl,-rpath,${YOLO11_ENV}/tensorrt-cpp/lib"
        "-Wl,-rpath,${YOLO11_ENV}/targets/x86_64-linux/lib")
```

以后添加源文件时：

```cmake
add_executable(detector_app src/main.cpp)
target_link_libraries(detector_app PRIVATE yolo_dependencies)
```

CLion 重新加载：

- 右键 `CMakeLists.txt` → **Reload CMake Project**（或 `File → Reload CMake Project`）。
- 如果仍看到旧的 `Specify CUDA_TOOLKIT_ROOT_DIR` 错误，删除 `cmake-build-debug/CMakeCache.txt`（或整个 `cmake-build-debug`）后再 Reload，让新配置重新生成缓存。

说明：

- 可用 `-DYOLO11_ENV=/path/to/env` 覆盖环境路径；CLion 在 `Settings → Build, Execution, Deployment → CMake → CMake options` 里填。
- `CMakeLists.txt` 会优先采用 `CONDA_PREFIX`（只有当 `$CONDA_PREFIX/tensorrt-cpp` 存在时），否则回退到本机 yolo11 路径。
- `cmake/FindTensorRT.cmake` / `FindNPP.cmake` 只是查找脚本，不含业务代码。
- 目前 `src/` 为空，所以构建显示 `ninja: no work to do` 是正常的；添加源文件后可正常编译。

### 6.11 USE_GPU_PREPROCESS 开关

`CMakeLists.txt` 中已加入默认开启的选项：

```cmake
option(USE_GPU_PREPROCESS "Enable GPU preprocessing (NPP/CUDA) in sources" ON)

if(USE_GPU_PREPROCESS)
    target_compile_definitions(yolo_dependencies INTERFACE USE_GPU_PREPROCESS)

    # 预处理的常用 NPP 组件：
    #   nppig   -> resize / warp
    #   nppidei -> copy / set / 初始化
    #   nppicc  -> 颜色转换
    foreach(_npp_component IN ITEMS
            NPP_NPPIG_LIBRARY NPP_NPPIDEI_LIBRARY NPP_NPPICC_LIBRARY)
        if(${_npp_component})
            target_link_libraries(yolo_dependencies INTERFACE "${${_npp_component}}")
        endif()
    endforeach()
endif()
```

行为：

- 默认 **ON**：所有链接 `yolo_dependencies` 的目标都会收到 `-DUSE_GPU_PREPROCESS`，并自动链接 NPP 的 geometry/data-exchange/color 组件。
- 关闭：在 CLion 的 `CMake options` 中加入 `-DUSE_GPU_PREPROCESS=OFF`，重新加载后即为 CPU 路径。
- 宏是编译期的；切换后需要重新配置/编译（CLion 会自动重新生成）。
- 当前 `src/` 为空，所以宏虽然已启用，但还没有业务代码引用；添加源文件并链接 `yolo_dependencies` 后生效。

代码中使用：

```cpp
#ifdef USE_GPU_PREPROCESS
    // NPP / CUDA 预处理路径（resize、颜色转换、归一化等）
#else
    // CPU 预处理路径
#endif
```

如果 GPU 预处理使用自定义 `.cu` 核函数，还需要把项目语言改成：

```cmake
project(detector LANGUAGES CXX CUDA)
set(CMAKE_CUDA_ARCHITECTURES 120)   # RTX 5060 = sm_120
```

---

## 7. 常用命令

```bash
# 激活环境
conda activate yolo11

# 版本
python -c "import torch, tensorrt, ultralytics; print(torch.__version__, tensorrt.__version__, ultralytics.__version__)"

# PyTorch 推理
python -c "from ultralytics import YOLO; m=YOLO('model/armor_best.pt'); print(m.predict('/tmp/yolo11_verify/bus.jpg', imgsz=640, device=0))"

# 导出 ONNX（默认写到模型同目录，建议先 copy 到 /tmp 再导出，避免覆盖原文件）
python -c "from ultralytics import YOLO; YOLO('/tmp/trt_export/armor_best.pt').export(format='onnx', imgsz=640, quantize=None, device=0)"

# 训练（示例）
yolo train model=yolo11n.pt data=/path/to/data.yaml epochs=100 imgsz=640 device=0

# TensorRT C++ 快速验证（本环境已生成的测试）
cd /tmp/yolo11_verify/cpp && ./build/trt_smoke
```

---

## 8. 已知问题与注意事项

1. **旧 engine 不可用**：`model/*.engine` 是 TensorRT 10.x 的，TensorRT 11.4 无法反序列化。必须在本机重建；原文件未改动。
2. **TensorRT pip 引导包坑**：直接 `pip install tensorrt` 可能因 3.8 GB wheel 下载截断、哈希校验失败而失败。使用第 2.2 节的真实 wheel 直链和 `--find-links`。
3. **Ultralytics 8.4 的 FP16 导出依赖 ModelOpt**：`quantize=16` 会拉 `nvidia-modelopt[onnx]`，并因 `torch==2.14.1+cu130` 的版本号问题自动安装失败。本环境用 ONNX Runtime float16 转换绕过，已验证。
4. **不要用 base 环境**：base 是 Python 3.14，这些包（尤其 TensorRT/Ultralytics）没有在该版本验证；始终 `conda activate yolo11`。
5. **数据集路径**：`config/*.yaml` 里的 `path` 指向 `/root/autodl-tmp/...` 或 `/home/pathos/桌面/...`，当前机器上不存在；训练前需要改成实际数据路径。本次只验证了推理、导出和合成数据训练 smoke test。
6. **显存**：8 GB。构建 1280 模型 engine 时建议逐个构建；如遇 OOM，把 `workspace` 从 4 GB 降到 2 GB。
7. **C++ 版本**：TensorRT 11 头文件要求 C++17；本机 g++ 11.4 + CMake 3.22 已通过。
8. **引擎可移植性**：engine 与 TensorRT 版本 + GPU 架构绑定；换机器/换版本必须重建。
9. **CUDA 版本混用**：Python 用 torch 的 cu130，C++ 用 conda 的 13.2.86；已验证兼容。如果升级 TensorRT，请保持 CUDA 13.x。
10. **临时文件**：验证脚本/日志在 `/tmp/yolo11_verify`，重启后可能被清理。核心构建脚本已完整收录在第 5.3 节。
11. **许可**：Ultralytics 是 AGPL-3.0；TensorRT 使用 NVIDIA TensorRT License。部署/分发前请确认许可要求。
12. **NPP 版本**：conda 的 NPP 13.2.x 要求 CUDA 13.4，会升级当前 CUDA 13.2 栈；本环境固定使用 **NPP 13.1.0.59**。NPP 13.1 没有 `libnppi.so` 总库，必须按模块链接（如 `nppig` + `nppidei` + `nppc`）；resize 等接口使用 `_Ctx` 版本，需要填 `NppStreamContext`。TensorRT 推理本身不依赖 NPP，只有做 GPU 图像预处理时才需要。
13. **CMake 4.x**：`FindCUDA` 已被移除，`find_package(CUDA)` 会报 `Specify CUDA_TOOLKIT_ROOT_DIR`。应改用 `find_package(CUDAToolkit)`（需要 nvcc）或显式 imported target；项目的 `CMakeLists.txt` 已迁移，TensorRT/NPP 通过 `cmake/FindTensorRT.cmake`、`cmake/FindNPP.cmake` 查找。不要再用 `find_package(CUDA)`。
14. **USE_GPU_PREPROCESS**：默认 ON，会定义宏并链接 NPP 的 `nppig/nppidei/nppicc`；CPU 路径用 `-DUSE_GPU_PREPROCESS=OFF`。当前 `src/` 为空，宏尚未被业务代码引用。
15. **TensorRT 11 的 `EXPLICIT_BATCH` 报错**：`NetworkDefinitionCreationFlag.EXPLICIT_BATCH`、`platform_has_fast_fp16`、`BuilderFlag.FP16` 都已在 TensorRT 11 中删除。用 `builder.create_network()`，FP16 通过把 ONNX 转成 FP16 来实现。`src/onnx_to_trt.py` 已迁移，详见 5.5。

---

## 9. 验证脚本与日志位置

```text
/tmp/yolo11_verify/
├── check_gpu.py              # GPU/PyTorch
├── check_yolo.py             # YOLO11 推理
├── check_train.py            # 训练 smoke test
├── check_trt.py              # FP32 TRT engine 构建/推理/旧 engine 兼容性
├── build_engine_fp16.py      # ONNX -> FP16 -> TensorRT engine（核心）
├── check_fp16_armor.py       # armor FP16 性能
├── check_fp16_yolo11n.py     # yolo11n FP32/FP16 精度对比
├── check_fp16_cls.py         # 分类模型 FP16
├── cpp/
│   ├── main.cpp              # Builder/Network/ONNX Parser smoke test
│   ├── infer_smoke.cpp       # 反序列化 FP16 engine + enqueueV3 推理
│   ├── npp_smoke.cpp         # NPP resize 验证
│   └── CMakeLists.txt        # 三个目标的完整 CMake 配置
└── *.log                     # 验证日志
```

pip 安装日志：

```text
/tmp/yolo11_pip_core.log
/tmp/yolo11_pip_install.log
```

如果重启后 `/tmp` 被清理，按第 3、5、6 节的命令可以完整复现。
