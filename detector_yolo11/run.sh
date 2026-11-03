#!/bin/bash

source "$HOME/anaconda3/etc/profile.d/conda.sh"
conda activate yolo11
python3 "$PWD/src/train.py"
