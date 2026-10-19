/*
 *  _   _  _______   _______   _____
 * | \ | ||  ___\ \ / /_   _| |  ___|
 * |  \| || |__  \ V /  | |   | |__
 * | . ` ||  __| /   \  | |   |  __|
 * | |\  || |___/ /^\ \ | |   | |___
 * \_| \_/\____/\/   \/ \_/   \____/
 *
 * @Author: pathos (王家辉)
 * @Date: 2026-10-08 08:13
 * @Description:
 *
 * Copyright (c) 2026 by XAUT NEXT-E/pathos.
 */

#include "hik_camera.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstring>
#include <sstream>
#include <opencv2/core.hpp>

namespace nexte {

    hik_camera::hik_camera(int devcice_number_):devcice_number(devcice_number_) {
        SDK_Init();
        Get_SDK_Version();
        Enumerate_Devices();
        Create_Camera_Instance(devcice_number_);
        Open_Camera();
    }


    void hik_camera::Get_SDK_Version() {
        const unsigned int version_sdk = MV_CC_GetSDKVersion();
        const unsigned int version_sdk_major = (version_sdk >> 24) & 0xFF;
        const unsigned int version_sdk_minor = (version_sdk >> 16) & 0xFF;
        const unsigned int version_sdk_revision = (version_sdk >> 8) & 0xFF;
        const unsigned int version_sdk_build = version_sdk  & 0xFF;
        std::ostringstream version_sdk_ostringstream;
        version_sdk_ostringstream << version_sdk_major <<"."<< version_sdk_minor <<"."<< version_sdk_revision <<"."<< version_sdk_build;
        version_sdk_string_mid = version_sdk_ostringstream.str();
        std::cout << "🟢当前SDK的版本是：" << version_sdk_string_mid << std::endl;
    }

    void hik_camera::SDK_Init() {
        int nRet;
        nRet = MV_CC_Initialize();
        if (nRet == MV_OK) {
            std::cout << "🟢SDK成功初始化" << std::endl;
        }
        else {
            std::cout << "🔴SDK失败初始化，状态码是："<< nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::SDK_Final() {
        int nRet;
        nRet = MV_CC_Finalize();
        if (nRet == MV_OK) {
            std::cout << "🟢SDK成功终止" << std::endl;
        }
        else {
            std::cout << "🔴SDK失败终止,状态码是："<< nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Enumerate_Devices() {
        int nRet;
        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE|MV_USB_DEVICE, &stDeviceList);
        if (nRet == MV_OK) {
            std::cout << "🟢设备枚举成功" << std::endl;
            std::cout << "🟢枚举出："<< stDeviceList.nDeviceNum<<"台设备" << std::endl;
            for (unsigned int i = 0; i < stDeviceList.nDeviceNum; ++i) {
                MV_CC_DEVICE_INFO* device_info = stDeviceList.pDeviceInfo[i];
                switch (device_info -> nTLayerType) {
                    case 4:
                        std::cout << "🟢枚举出的第"<< i+1 <<"台相机是MV_USB_DEVICE" << std::endl;
                        std::cout << "🟢枚举出的第"<< i+1 <<"台相机的型号是" << device_info -> SpecialInfo.stUsb3VInfo.chModelName <<std::endl;
                        std::cout << "🟢枚举出的第"<< i+1 <<"台相机的序列号是" << device_info -> SpecialInfo.stUsb3VInfo.chSerialNumber <<std::endl;
                        break;
                    default:
                        std::cout << "🟢枚举出的第"<< i+1 <<"台相机是其他设备" << std::endl;
                        break;
                }
            }

        }
        else {
            std::cout << "🔴设备枚举失败，状态码是："<< nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        stDeviceList_mid = stDeviceList;
    }

    void hik_camera::Create_Camera_Instance(unsigned int nIndex) {
        int nRet;
        void* handle = nullptr;
        nRet = MV_CC_CreateHandle(&handle, stDeviceList_mid.pDeviceInfo[nIndex]);
        if (nRet == MV_OK) {
            std::cout << "🟢句柄初始化成功，初始化的是第"<< nIndex+1 << "台相机" << std::endl;
            handle_mid = handle;
        }
        else {
            std::cout << "🔴句柄初始化失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Destroy_Camera_Instance() {
        int nRet;
        void* handle = nullptr;
        nRet = MV_CC_DestroyHandle(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢句柄销毁成功" << std::endl;
            handle_mid = handle;
        }
        else {
            std::cout << "🔴句柄销毁失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Open_Camera() {
        int nRet;
        nRet = MV_CC_OpenDevice(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢成功打开相机" << std::endl;
        }
        else {
            std::cout << "🔴打开相机失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Close_Camera() {
        int nRet;
        nRet = MV_CC_CloseDevice(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢成功关闭相机" << std::endl;
        }
        else {
            std::cout << "🔴关闭相机失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Exposure(float exposure_us) {
        int nRet;
        unsigned int nValue = 0;
        nRet = MV_CC_SetEnumValue(handle_mid, "ExposureMode", nValue);
        if (nRet != MV_OK) {
            std::cout << "🔴曝光模式设置失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nRet = MV_CC_SetFloatValue(handle_mid, "ExposureTime", exposure_us);
        if (nRet == MV_OK) {
            std::cout << "🟢相机的曝光时间设置为 "<< exposure_us << " 微秒" << std::endl;
            exposure_time_mid = exposure_us;
        }
        else {
            std::cout << "🔴曝光时间设置失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Gain(float gain_db) {
        int nRet;
        nRet = MV_CC_SetFloatValue(handle_mid, "Gain", gain_db);
        if (nRet == MV_OK) {
            std::cout << "🟢相机增益设置为 "<< gain_db << " db" << std::endl;
            gain_mid = gain_db;
        }
        else {
            std::cout << "🔴相机增益设置失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Pixel_Format() {
        int nRet;
        nRet = MV_CC_SetEnumValueByString(handle_mid, "PixelFormat", "BayerGB8");
        if (nRet == MV_OK) {
            std::cout << "🟢像素格式成功设置" << std::endl;
        }
        else {
            std::cout << "🔴像素格式失败设置，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Frame_Rate(float fps) {
        int nRet;
        nRet = MV_CC_SetBoolValue(handle_mid,"AcquisitionFrameRateEnable",true);
        if (nRet == MV_OK) {
            std::cout << "🟢开启了帧率控制" << std::endl;
        }
        else {
            std::cout << "🔴开启帧率控制失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nRet = MV_CC_SetFloatValue(handle_mid,"AcquisitionFrameRate",fps);
        if (nRet == MV_OK) {
            std::cout << "🟢成功设置了帧率 " << fps << std::endl;
            fps_mid = fps;
        }
        else {
            std::cout << "🔴开启帧率控制失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Open_AcquisitionMode_Mode() {
        int nRet;
        nRet = MV_CC_SetEnumValueByString(handle_mid, "AcquisitionMode", "Continuous");
        if (nRet == MV_OK) {
            std::cout << "🟢成功将采集模式设置为连续采集" << std::endl;
        }
        else {
            std::cout << "🔴采集模式设置失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nRet = MV_CC_SetEnumValueByString(handle_mid, "TriggerMode", "Off");
        if (nRet == MV_OK) {
            std::cout << "🟢成功关闭了触发模式" << std::endl;
        }
        else {
            std::cout << "🔴关闭了触发失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Get_PayloadSize_Alignment() {
        int nRet;
        nRet = MV_CC_GetPayloadSize(handle_mid, &sdk_payload_size_mid, &sdk_alignment_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢负载设置成功，负载长度为 "<< sdk_payload_size_mid << " 字节" << std::endl;
            std::cout << "🟢负载设置成功，对齐方式是 "<< sdk_alignment_mid << " 字节" << std::endl;
        }
        else {
            std::cout << "🔴负载设置失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Get_AllocAlignedBuffer() {
        sdk_allocate_aligned_buffer_mid = MV_CC_AllocAlignedBuffer(sdk_payload_size_mid, sdk_alignment_mid);
        if (sdk_allocate_aligned_buffer_mid == nullptr) {
            std::cout << "🔴缓存配置失败" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        else {
            std::cout << "🟢缓存配置成功，地址为 "<< sdk_allocate_aligned_buffer_mid << std::endl;
        }
    }

    void hik_camera::Free_AllocAlignedBuffer() {
        int nRet;
        nRet = MV_CC_FreeAlignedBuffer(sdk_allocate_aligned_buffer_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢缓冲区释放成功"<< std::endl;
        }
        else {
            std::cout << "🔴缓冲区释放失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Register_Buffer() {
        int nRet;
        nRet = MV_CC_RegisterBuffer(handle_mid, sdk_allocate_aligned_buffer_mid, sdk_payload_size_mid, nullptr);
        if (nRet == MV_OK) {
            std::cout << "🟢缓冲区注册成功"<< std::endl;
        }
        else {
            std::cout << "🔴缓冲区注册失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Unregister_Buffer() {
        int nRet;
        nRet = MV_CC_UnRegisterBuffer(handle_mid, sdk_allocate_aligned_buffer_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢缓冲区内存取消注册成功"<< std::endl;
        }
        else {
            std::cout << "🔴缓冲区内存取消注册失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Image_Node_Number() {
        int nRet;
        nRet = MV_CC_SetImageNodeNum(handle_mid, image_node_number);;
        if (nRet == MV_OK) {
            std::cout << "🟢设置SDK内部节点成功"<< std::endl;
        }
        else {
            std::cout << "🔴设置SDK内部节点失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nRet = MV_CC_SetGrabStrategy(handle_mid, MV_GrabStrategy_OneByOne );
        if (nRet != MV_OK) {
            std::cout << "🔴设置SDK内部节点永远取最新帧失败，状态码是：" << nRet << std::endl;
        }
    }



    void hik_camera::Start_Grabbing() {
        int nRet;
        nRet = MV_CC_StartGrabbing(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢开始取流成功"<< std::endl;
        }
        else {
            std::cout << "🔴开始取流失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }

    }

    //取最新的一帧的回调函数
    void __stdcall hik_camera::capture_latest_frame(unsigned char* pData,
                                        MV_FRAME_OUT_INFO_EX* pFrameInfo,
                                        void* pUser) {
        std::cout << "🟢停止该帧的本机时间戳为 "<< pFrameInfo->nHostTimeStamp  <<
            ",该帧的帧号为 "<<  pFrameInfo->nFrameNum <<std::endl;

        cv::Mat this_image(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);
        auto* camera = static_cast<hik_camera*>(pUser);
        camera -> frame.latest_Image = this_image.clone();
    }

    void hik_camera::register_image_callback() {

        int nRet = MV_CC_RegisterImageCallBackEx(handle_mid,&capture_latest_frame, this);
        if (nRet != MV_OK) {
            std::cerr << "🔴注册回调失败：" << nRet << '\n';
            std::exit(EXIT_FAILURE);
        }
    }



    void hik_camera::Stop_Grabbing() {
        int nRet;
        nRet = MV_CC_StopGrabbing(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢停止取流成功"<< std::endl;
        }
        else {
            std::cout << "🔴停止取流失败，状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    hik_camera::~hik_camera() {
        Stop_Grabbing();
        //camera.Unregister_Buffer();
        //camera.Free_AllocAlignedBuffer();
        Close_Camera();
        Destroy_Camera_Instance();
        SDK_Final();
    }



}
