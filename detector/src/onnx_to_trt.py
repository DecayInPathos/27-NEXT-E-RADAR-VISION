#
#  _   _  _______   _______   _____
# | \ | ||  ___\ \ / /_   _| |  ___|
# |  \| || |__  \ V /  | |   | |__
# | . ` ||  __| /   \  | |   |  __|
# | |\  || |___/ /^\ \ | |   | |___
# \_| \_/\____/\/   \/ \_/   \____/
#
# @Author: pathos (王家辉)
# @Date: 2026-10-20 19:30
# @Description: ONNX 模型转换为 TensorRT 推理引擎
#
# Copyright (c) 2026 by XAUT NEXT-E/pathos.
# 参考：https://zhuanlan.zhihu.com/p/13772766755
#

import ast
import json
import os
import struct
import sys
import tempfile
from pathlib import Path

import onnx
import tensorrt as trt
from onnxruntime.transformers.float16 import convert_float_to_float16

BASE_DIR = Path(__file__).resolve().parent.parent


def get_ultralytics_metadata(onnx_model, fp16_mode):
    """Build the same JSON metadata Ultralytics writes into an .engine file.

    Ultralytics ONNX exports store description/task/imgsz/names/args in
    metadata_props. The engine header is:
        4-byte little-endian JSON length + JSON + serialized TensorRT engine
    """
    props = {kv.key: kv.value for kv in onnx_model.metadata_props}
    if not props:
        return None

    metadata = {}
    for key, value in props.items():
        if key in {"stride", "batch", "channels"}:
            metadata[key] = int(value)
        elif key in {"imgsz", "names", "args"}:
            metadata[key] = ast.literal_eval(value)  # these are Python literals, not JSON
        else:
            metadata[key] = value

    args = metadata.get("args")
    if isinstance(args, dict):
        args["half"] = bool(fp16_mode)  # reflect the actual engine precision

    return metadata


def build_engine(onnx_file_path, trt_model_path, max_workspace_size=4 << 30, fp16_mode=True):
    """Build a TensorRT 11 engine from an ONNX model.

    Outputs:
      * <trt_model_path>             raw serialized engine (for custom C++ runtime)
      * <trt_model_path>.engine      same engine + Ultralytics metadata header
    """
    onnx_file_path = Path(onnx_file_path)
    trt_model_path = Path(trt_model_path)
    trt_model_path.parent.mkdir(parents=True, exist_ok=True)
    engine_file_path = trt_model_path.with_suffix(".engine")

    logger = trt.Logger(trt.Logger.WARNING)
    trt.init_libnvinfer_plugins(logger, "")

    builder = trt.Builder(logger)
    network = builder.create_network()  # TensorRT 11: explicit batch is the only mode

    onnx_model = onnx.load(str(onnx_file_path))
    metadata = get_ultralytics_metadata(onnx_model, fp16_mode)

    parse_path = onnx_file_path
    fp16_onnx_path = None
    if fp16_mode:
        # TensorRT 11 has no BuilderFlag.FP16; bake FP16 into the ONNX.
        model_fp16 = convert_float_to_float16(onnx_model, keep_io_types=True)
        fd, fp16_name = tempfile.mkstemp(suffix=".fp16.onnx")
        os.close(fd)
        fp16_onnx_path = Path(fp16_name)
        onnx.save(model_fp16, str(fp16_onnx_path))
        parse_path = fp16_onnx_path
        print(f"FP16 ONNX: {fp16_onnx_path}")

    parser = trt.OnnxParser(network, logger)
    try:
        with open(parse_path, "rb") as model_file:
            if not parser.parse(model_file.read()):
                print("ERROR: Failed to parse the ONNX file.")
                for error in range(parser.num_errors):
                    print(parser.get_error(error))
                return None
    finally:
        if fp16_onnx_path is not None:
            fp16_onnx_path.unlink(missing_ok=True)

    input_tensor = network.get_input(0)
    print(f"network input: {input_tensor.name} {tuple(input_tensor.shape)}")

    config = builder.create_builder_config()
    config.set_memory_pool_limit(trt.MemoryPoolType.WORKSPACE, max_workspace_size)

    # All models in this project have static input shapes, so no optimization
    # profile is needed. For a dynamic-shape ONNX, use the real input name:
    #
    #   profile = builder.create_optimization_profile()
    #   profile.set_shape(network.get_input(0).name, min_shape, opt_shape, max_shape)
    #   config.add_optimization_profile(profile)

    serialized_engine = builder.build_serialized_network(network, config)
    if serialized_engine is None:
        print("Failed to build serialized engine.")
        return None

    # 1) Raw engine, no metadata: IRuntime::deserializeCudaEngine can load it directly.
    with open(trt_model_path, "wb") as f:
        f.write(serialized_engine)
    print(f"raw TensorRT engine: {trt_model_path}")

    # 2) Ultralytics-compatible engine: int32 JSON length + JSON + engine bytes.
    if metadata is not None:
        header = json.dumps(metadata, ensure_ascii=False).encode("utf-8")
        with open(engine_file_path, "wb") as f:
            f.write(struct.pack("<i", len(header)))
            f.write(header)
            f.write(serialized_engine)
        print(f"Ultralytics engine: {engine_file_path} "
              f"(metadata {len(header)} bytes, task={metadata.get('task')}, "
              f"imgsz={metadata.get('imgsz')}, classes={len(metadata.get('names', {}))})")
    else:
        print("WARNING: ONNX has no metadata_props; only the raw engine was written.")

    runtime = trt.Runtime(logger)
    engine = runtime.deserialize_cuda_engine(serialized_engine)
    return engine


def main():
    if len(sys.argv) >= 2:
        onnx_model_path = Path(sys.argv[1])
        if len(sys.argv) >= 3:
            trt_model_path = Path(sys.argv[2])
        else:
            trt_model_path = BASE_DIR / "model/trt" / f"{onnx_model_path.stem}.trt"
    else:
        onnx_model_path = BASE_DIR / "model/onnx/armor_best_640.onnx"
        trt_model_path = BASE_DIR / "model/trt/armor_best_640.trt"

    engine = build_engine(str(onnx_model_path), str(trt_model_path), fp16_mode=True)
    if engine is None:
        raise SystemExit(1)

    print("engine inputs/outputs:")
    for i in range(engine.num_io_tensors):
        name = engine.get_tensor_name(i)
        print(f"  {name}: {engine.get_tensor_shape(name)} {engine.get_tensor_dtype(name)}")


if __name__ == "__main__":
    main()
