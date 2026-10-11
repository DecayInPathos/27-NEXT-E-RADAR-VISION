/*
 *  _   _  _______   _______   _____
 * | \ | ||  ___\ \ / /_   _| |  ___|
 * |  \| || |__  \ V /  | |   | |__
 * | . ` ||  __| /   \  | |   |  __|
 * | |\  || |___/ /^\ \ | |   | |___
 * \_| \_/\____/\/   \/ \_/   \____/
 *
 * @Author: pathos (王家辉)
 * @Date: 2026-10-08 19:49
 * @Description:
 *
 * Copyright (c) 2026 by XAUT NEXT-E/pathos.
 */

#pragma once

#include "MvCameraControl.h"
#include <iostream>
#include <sstream>
#include <string>

namespace nexte {
    class hik_camera {
    private:
        // SDK版本变量
        std::string version_sdk_string_mid;

        // 相机枚举的设备信息
        MV_CC_DEVICE_INFO_LIST stDeviceList_mid;

        // 设备实例的句柄
        void* handle_mid;

        // 设备曝光时间，默认38000.0000
        float exposure_time_mid = 38000.0000;

        // 设备增益，默认0
        float gain_mid = 0.0000;

        // SDK所需要的缓存区的大小
        uint64_t sdk_payload_size_mid = 0;

        // sdk内存对齐要求
        unsigned int sdk_alignment_mid = 0;

        // sdk分配缓存的地址
        void* sdk_allocate_aligned_buffer_mid = nullptr;

        // 帧率控制目标，默认为150
        float fps_mid = 150.0000;

    public:
        // 获取SDK版本,作为初始化测试
        void Get_SDK_Version(void);

        // SDK初始化
        void SDK_Init(void);

        // SDK终止化
        void SDK_Final(void);

        // 枚举设备
        void  Enumerate_Devices(void);

        // 创建相机实例
        void Create_Camera_Instance(unsigned int nIndex);

        // 销毁相机实例
        void Destroy_Camera_Instance(void);

        // 打开相机
        void Open_Camera(void);

        // 关闭相机
        void Close_Camera(void);

        // 设置曝光时间，单位：微秒
        // 学长给我讲都是手动设置，所以只保留手动设置
        void Set_Exposure(float exposure_us);

        // 设置增益，单位：dB
        void Set_Gain(float gain_db);

        // 设置相机输出的像素格式
        void Set_Pixel_Format(void);

        // 启用帧率控制并设置目标帧率，单位：帧/秒
        void Set_Frame_Rate(float fps);

        // 设置连续采集模式：Off、On
        void Open_AcquisitionMode_Mode(void);

        // 缓冲区获取SDK所需要的缓存区的大小，和内存对齐方式
        // 将值赋给 sdk_payload_size_mid sdk_alignment_mid
        void Get_PayloadSize_Alignment(void);

        // 分配缓冲区缓存地址
        void Get_AllocAlignedBuffer(void);

        // 释放缓冲区缓存地址
        void Free_AllocAlignedBuffer(void);

        // 缓冲区注册内存
        void Register_Buffer(void);

        // 缓冲区内存取消注册
        void Unregister_Buffer(void);

        // 开始取流
        void Start_Grabbing(void);

        // 停止取流
        void Stop_Grabbing(void);




    };
}
