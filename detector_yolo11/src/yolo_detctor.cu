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

#include "yolo_detctor.cuh"
#include <iostream>
#include <npp.h>
#include <algorithm>
#include <cmath>
nexte::yolo_detctor::yolo_detctor(cv::Mat host_raw_image_,int image_size_):
host_raw_image(host_raw_image_),image_size(image_size_){
    try {
        device_copy_image.upload(host_raw_image);
        device_RGB_image.create(device_copy_image.rows,device_copy_image.cols,CV_8UC3);
        scale_factor_rows = static_cast<double>(image_size) / device_RGB_image.rows ;
        scale_factor_cols = static_cast<double>(image_size) / device_RGB_image.cols;
        scale_factor = std::min(device_RGB_image.rows,device_RGB_image.cols);
        device_change_size_image.create(
        static_cast<int>(std::round(device_RGB_image.rows * scale_factor)),
        static_cast<int>(std::round(device_RGB_image.cols * scale_factor)),
            CV_8UC3);
        device_imput_image.create(640,640,CV_8UC3);
    }
    catch (const std::exception& e) {
        std::cout << "🔴失败初始化" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    cudaGetDevice(&ctx.nCudaDeviceId);
    cudaDeviceProp prop{};
    cudaGetDeviceProperties(&prop, ctx.nCudaDeviceId);
    ctx.nMultiProcessorCount = prop.multiProcessorCount;
    ctx.nMaxThreadsPerMultiProcessor = prop.maxThreadsPerMultiProcessor;
    ctx.nMaxThreadsPerBlock = prop.maxThreadsPerBlock;
    ctx.nSharedMemPerBlock = prop.sharedMemPerBlock;
    ctx.nCudaDevAttrComputeCapabilityMajor = prop.major;
    ctx.nCudaDevAttrComputeCapabilityMinor = prop.minor;
    ctx.hStream = nullptr;
    cudaStreamGetFlags(ctx.hStream, &ctx.nStreamFlags);
}

void nexte::yolo_detctor::preprocess() {
    nppiCFAToRGB_8u_C1C3R_Ctx(
        device_copy_image.data,
        static_cast<int>(device_copy_image.step),
        {device_copy_image.cols, device_copy_image.rows},
        {0, 0, device_copy_image.cols, device_copy_image.rows},
        device_RGB_image.data,
        static_cast<int>(device_RGB_image.step),
        NPPI_BAYER_GBRG,
        NPPI_INTER_UNDEFINED,
        ctx);

    NppStatus status = nppiResize_8u_C3R_Ctx(
        device_RGB_image.data,
        static_cast<int>(device_RGB_image.step),
        {device_RGB_image.cols, device_RGB_image.rows},
        {0, 0, device_RGB_image.cols, device_RGB_image.rows},
        device_change_size_image.data,
        static_cast<int>(device_change_size_image.step),
        {device_change_size_image.cols, device_change_size_image.rows},
        {0, 0, device_change_size_image.cols, device_change_size_image.rows},
        NPPI_INTER_LINEAR,
        ctx
    );



}





