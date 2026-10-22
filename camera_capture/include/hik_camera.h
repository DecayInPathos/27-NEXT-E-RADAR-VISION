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
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace nexte {
    class hik_camera {
    public:
        // 设备实例的句柄
        void* handle_mid;

    
    private:
        //设备编号，启动哪一台相机,从0开始
        int devcice_number = 0;

        // SDK版本变量
        std::string version_sdk_string_mid;

        // 相机枚举的设备信息
        MV_CC_DEVICE_INFO_LIST stDeviceList_mid;



        // 设备曝光时间，默认38000.0000
        float exposure_time_mid = 38000.0000;

        // 设备增益，默认0
        float gain_mid = 0.0000;

        // SDK所需要的缓存区的大小
        uint64_t sdk_payload_size_mid = 0;

        // sdk内存对齐要求
        unsigned int sdk_alignment_mid = 0;

        // sdk分配外部缓存的地址
        void* sdk_allocate_aligned_buffer_mid = nullptr;

        // 帧率控制目标，默认为150
        float fps_mid = 150.0000;

        // 图像节点数量设置为1
        unsigned int image_node_number = 1;

        //规定最新一帧的数据结构
        struct latest_frame {
            int64_t nHostTimeStamp;
            enum MvGvspPixelType enPixelType;
            cv::Mat latest_Image;
        };

        latest_frame frame;

    public:
        hik_camera(int devcice_number_);

        // 获取SDK版本,作为初始化测试
        void Get_SDK_Version();

        // SDK初始化
        void SDK_Init();

        // SDK终止化
        void SDK_Final();

        // 枚举设备
        void  Enumerate_Devices();

        // 创建相机实例
        void Create_Camera_Instance(unsigned int nIndex);

        // 销毁相机实例
        void Destroy_Camera_Instance();

        // 打开相机
        void Open_Camera();

        // 关闭相机
        void Close_Camera();

        // 设置曝光时间，单位：微秒
        // 学长给我讲都是手动设置，所以只保留手动设置
        void Set_Exposure(float exposure_us);

        // 设置增益，单位：dB
        void Set_Gain(float gain_db);

        // 设置相机输出的像素格式
        void Set_Pixel_Format();

        // 启用帧率控制并设置目标帧率，单位：帧/秒
        void Set_Frame_Rate(float fps);

        // 设置连续采集模式：Off、On
        void Open_AcquisitionMode_Mode();

        // 缓冲区获取SDK所需要的缓存区的大小，和内存对齐方式
        // 将值赋给 sdk_payload_size_mid sdk_alignment_mid
        void Get_PayloadSize_Alignment();

        // 分配缓冲区缓存地址
        void Get_AllocAlignedBuffer();

        // 释放缓冲区缓存地址
        void Free_AllocAlignedBuffer();

        // 缓冲区注册内存
        void Register_Buffer();

        // 缓冲区内存取消注册
        void Unregister_Buffer();

        //设置SDK内部的节点数量为1，而且确保策略是新帧覆盖旧帧
        void Set_Image_Node_Number();



        //取最新的一帧的回调函数
        static void __stdcall capture_latest_frame(unsigned char* pData,
                                            MV_FRAME_OUT_INFO_EX* pFrameInfo,
                                            void* pUser);

        void register_image_callback();

        // 开始取流
        void Start_Grabbing();
        // 停止取流
        void Stop_Grabbing();

        
        ~hik_camera();

    };
}
