/*
 *  _   _  _______   _______   _____
 * | \ | ||  ___\ \ / /_   _| |  ___|
 * |  \| || |__  \ V /  | |   | |__
 * | . ` ||  __| /   \  | |   |  __|
 * | |\  || |___/ /^\ \ | |   | |___
 * \_| \_/\____/\/   \/ \_/   \____/
 *
 * @Author: pathos (王家辉)
 * @Date: 2026-10-21 11:44
 * @Description: yolov11n识别器的实现
 *
 * Copyright (c) 2026 by XAUT NEXT-E/pathos.
 */

#pragma once
#include <npp.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>


namespace nexte {
    class yolo_detctor {
    private:
        cv::Mat host_raw_image;
        cv::cuda::GpuMat device_copy_image;
        cv::cuda::GpuMat device_RGB_image;
        NppStreamContext ctx{};
        int image_size;
        double scale_factor_rows;
        double scale_factor_cols;
        double scale_factor;
        cv::cuda::GpuMat device_change_size_image;
        cv::cuda::GpuMat device_imput_image;
        const Npp8u padding[3] = {114, 114, 114};



    public:
        yolo_detctor(cv::Mat host_raw_image_, int image_size_);
        void preprocess();
        ~yolo_detctor();
    };
}