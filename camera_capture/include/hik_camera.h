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
        //SDK版本变量
        const unsigned int version_sdk = MV_CC_GetSDKVersion();
        const unsigned int version_sdk_major = (version_sdk >> 24) & 0xFF;
        const unsigned int version_sdk_minor = (version_sdk >> 16) & 0xFF;
        const unsigned int version_sdk_revision = (version_sdk >> 8) & 0xFF;
        const unsigned int version_sdk_build = version_sdk  & 0xFF;

        //相机枚举的设备信息
        MV_CC_DEVICE_INFO_LIST stDeviceList_mid;

        //设备实例的句柄
        void* handle_mid;

    public:
        //获取SDK版本
        std::string Get_SDK_Version(void);

        //SDK初始化
        int SDK_Init(void);

        //SDK终止化
        int SDK_Final(void);

        //枚举设备
        MV_CC_DEVICE_INFO_LIST Enumerate_Devices(void);

        //创建相机实例
        void Create_Camera_Instance(unsigned int nIndex);

        //销毁相机实例
        void Destroy_Camera_Instance(void);

        //打开相机
        void Open_Camera(void);

        //关闭相机
        void Close_Camera(void);

        // 设置曝光时间，单位：微秒
        void Set_Exposure(float exposure_us);

        // 设置自动曝光模式：Off、Once、Continuous
        void Set_Auto_Exposure(const std::string& mode);

        // 设置自动曝光时间上下限，单位：微秒
        void Set_Auto_Exposure_Limits(float min_us, float max_us);

        // 设置增益，单位：dB
        void Set_Gain(float gain_db);

        // 设置自动白平衡模式：Off、Once、Continuous
        void Set_Auto_White_Balance(const std::string& mode);

        // 设置采集区域，单位：像素
        void Set_ROI(unsigned int width, unsigned int height,
                    unsigned int offset_x, unsigned int offset_y);

        // 设置相机输出的像素格式
        void Set_Pixel_Format(unsigned int format);

        // 启用帧率控制并设置目标帧率，单位：帧/秒
        void Set_Frame_Rate(float fps);

        // 设置触发模式：Off、On
        void Set_Trigger_Mode(const std::string& mode);

        // 设置触发源，例如 Software、Line0
        void Set_Trigger_Source(const std::string& source);

        // 执行一次软件触发
        void Software_Trigger();
    };
}
