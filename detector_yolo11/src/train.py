#
#  _   _  _______   _______   _____
# | \ | ||  ___\ \ / /_   _| |  ___|
# |  \| || |__  \ V /  | |   | |__
# | . ` ||  __| /   \  | |   |  __|
# | |\  || |___/ /^\ \ | |   | |___
# \_| \_/\____/\/   \/ \_/   \____/
#
# @Author: pathos (王家辉)
# @Date: 2026-11-1 18:14
# @Description: ONNX 模型转换为 TensorRT 推理引擎
#
# Copyright (c) 2026 by XAUT NEXT-E/pathos.
#

import sys
from pathlib import Path
ultralytics_path = Path(__file__).resolve().parents[2] / "ultralytics"
sys.path.insert(0, ultralytics_path)

try:
    from ultralytics import YOLO
    print("🟢ultralytics导入成功")
except BaseException:
    print("🔴ultralytics导入失败")

model = YOLO("yolo11n.yaml")

model.val()

results = model.train(data="coco8.yaml", epochs=400, imgsz=640, device=[0,1])


