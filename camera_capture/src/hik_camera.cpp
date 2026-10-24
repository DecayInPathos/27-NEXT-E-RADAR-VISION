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
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>

namespace nexte {

    hik_camera::hik_camera(int devcice_number_):devcice_number(devcice_number_) {
        Get_Sdk_Version();
        Enumerate_Devices();
        Create_Camera_Instance(devcice_number_);
        Open_Camera();
        Set_Exposure(exposure_time_mid);
        Set_Gain(0);
        Set_Pixel_Format();
        Set_Frame_Rate(100.0000);
        Open_AcquisitionMode_Mode();
        //camera.Get_PayloadSize_Alignment();
        //camera.Get_AllocAlignedBuffer();
        //camera.Register_Buffer();
        Set_Image_Node_Number();
        Register_Image_Callback();
        Start_Grabbing();
    }


    void hik_camera::Get_Sdk_Version() {
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

    void hik_camera::Sdk_Init() {
        int nret;
        nret = MV_CC_Initialize();
        if (nret == MV_OK) {
            std::cout << "🟢SDK成功初始化" << std::endl;
        }
        else {
            std::cout << "🔴SDK失败初始化，状态码是："<< nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Sdk_Final() {
        int nret;
        nret = MV_CC_Finalize();
        if (nret == MV_OK) {
            std::cout << "🟢SDK成功终止" << std::endl;
        }
        else {
            std::cout << "🔴SDK失败终止,状态码是："<< nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Enumerate_Devices() {
        int nret;
        MV_CC_DEVICE_INFO_LIST stdevicelist;
        memset(&stdevicelist, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        nret = MV_CC_EnumDevices(MV_GIGE_DEVICE|MV_USB_DEVICE, &stdevicelist);
        if (nret == MV_OK) {
            std::cout << "🟢设备枚举成功" << std::endl;
            std::cout << "🟢枚举出："<< stdevicelist.nDeviceNum<<"台设备" << std::endl;
            for (unsigned int i = 0; i < stdevicelist.nDeviceNum; ++i) {
                MV_CC_DEVICE_INFO* device_info = stdevicelist.pDeviceInfo[i];
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
            std::cout << "🔴设备枚举失败，状态码是："<< nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
        stdevicelist_mid = stdevicelist;
    }

    void hik_camera::Create_Camera_Instance(unsigned int nindex) {
        int nret;
        void* handle = nullptr;
        nret = MV_CC_CreateHandle(&handle, stdevicelist_mid.pDeviceInfo[nindex]);
        if (nret == MV_OK) {
            std::cout << "🟢句柄初始化成功，初始化的是第"<< nindex+1 << "台相机" << std::endl;
            handle_mid = handle;
        }
        else {
            std::cout << "🔴句柄初始化失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Destroy_Camera_Instance() {
        int nret;
        void* handle = nullptr;
        nret = MV_CC_DestroyHandle(handle_mid);
        if (nret == MV_OK) {
            std::cout << "🟢句柄销毁成功" << std::endl;
            handle_mid = handle;
        }
        else {
            std::cout << "🔴句柄销毁失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Open_Camera() {
        int nret;
        nret = MV_CC_OpenDevice(handle_mid);
        if (nret == MV_OK) {
            std::cout << "🟢成功打开相机" << std::endl;
        }
        else {
            std::cout << "🔴打开相机失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Close_Camera() {
        int nret;
        nret = MV_CC_CloseDevice(handle_mid);
        if (nret == MV_OK) {
            std::cout << "🟢成功关闭相机" << std::endl;
        }
        else {
            std::cout << "🔴关闭相机失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Exposure(float exposure_us) {
        int nret;
        unsigned int nvalue = 0;
        nret = MV_CC_SetEnumValue(handle_mid, "ExposureMode", nvalue);
        if (nret != MV_OK) {
            std::cout << "🔴曝光模式设置失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nret = MV_CC_SetFloatValue(handle_mid, "ExposureTime", exposure_us);
        if (nret == MV_OK) {
            std::cout << "🟢相机的曝光时间设置为 "<< exposure_us << " 微秒" << std::endl;
            exposure_time_mid = exposure_us;
        }
        else {
            std::cout << "🔴曝光时间设置失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Gain(float gain_db) {
        int nret;
        nret = MV_CC_SetFloatValue(handle_mid, "Gain", gain_db);
        if (nret == MV_OK) {
            std::cout << "🟢相机增益设置为 "<< gain_db << " db" << std::endl;
            gain_mid = gain_db;
        }
        else {
            std::cout << "🔴相机增益设置失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Pixel_Format() {
        int nret;
        nret = MV_CC_SetEnumValueByString(handle_mid, "PixelFormat", "BayerGB8");
        if (nret == MV_OK) {
            std::cout << "🟢像素格式成功设置" << std::endl;
        }
        else {
            std::cout << "🔴像素格式失败设置，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Frame_Rate(float fps) {
        int nret;
        nret = MV_CC_SetBoolValue(handle_mid,"AcquisitionFrameRateEnable",true);
        if (nret == MV_OK) {
            std::cout << "🟢开启了帧率控制" << std::endl;
        }
        else {
            std::cout << "🔴开启帧率控制失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nret = MV_CC_SetFloatValue(handle_mid,"AcquisitionFrameRate",fps);
        if (nret == MV_OK) {
            std::cout << "🟢成功设置了帧率 " << fps << std::endl;
            fps_mid = fps;
        }
        else {
            std::cout << "🔴开启帧率控制失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Open_AcquisitionMode_Mode() {
        int nret;
        nret = MV_CC_SetEnumValueByString(handle_mid, "AcquisitionMode", "Continuous");
        if (nret == MV_OK) {
            std::cout << "🟢成功将采集模式设置为连续采集" << std::endl;
        }
        else {
            std::cout << "🔴采集模式设置失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nret = MV_CC_SetEnumValueByString(handle_mid, "TriggerMode", "Off");
        if (nret == MV_OK) {
            std::cout << "🟢成功关闭了触发模式" << std::endl;
        }
        else {
            std::cout << "🔴关闭了触发失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Get_PayloadSize_Alignment() {
        int nret;
        nret = MV_CC_GetPayloadSize(handle_mid, &sdk_payload_size_mid, &sdk_alignment_mid);
        if (nret == MV_OK) {
            std::cout << "🟢负载设置成功，负载长度为 "<< sdk_payload_size_mid << " 字节" << std::endl;
            std::cout << "🟢负载设置成功，对齐方式是 "<< sdk_alignment_mid << " 字节" << std::endl;
        }
        else {
            std::cout << "🔴负载设置失败，状态码是：" << nret << std::endl;
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
        int nret;
        nret = MV_CC_FreeAlignedBuffer(sdk_allocate_aligned_buffer_mid);
        if (nret == MV_OK) {
            std::cout << "🟢缓冲区释放成功"<< std::endl;
        }
        else {
            std::cout << "🔴缓冲区释放失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Register_Buffer() {
        int nret;
        nret = MV_CC_RegisterBuffer(handle_mid, sdk_allocate_aligned_buffer_mid, sdk_payload_size_mid, nullptr);
        if (nret == MV_OK) {
            std::cout << "🟢缓冲区注册成功"<< std::endl;
        }
        else {
            std::cout << "🔴缓冲区注册失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Unregister_Buffer() {
        int nret;
        nret = MV_CC_UnRegisterBuffer(handle_mid, sdk_allocate_aligned_buffer_mid);
        if (nret == MV_OK) {
            std::cout << "🟢缓冲区内存取消注册成功"<< std::endl;
        }
        else {
            std::cout << "🔴缓冲区内存取消注册失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Set_Image_Node_Number() {
        int nret;
        nret = MV_CC_SetImageNodeNum(handle_mid, image_node_number);;
        if (nret == MV_OK) {
            std::cout << "🟢设置SDK内部节点成功"<< std::endl;
        }
        else {
            std::cout << "🔴设置SDK内部节点失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
        nret = MV_CC_SetGrabStrategy(handle_mid, MV_GrabStrategy_OneByOne );
        if (nret != MV_OK) {
            std::cout << "🔴设置SDK内部节点永远取最新帧失败，状态码是：" << nret << std::endl;
        }
    }

    void hik_camera::Start_Grabbing() {
        int nret;
        nret = MV_CC_StartGrabbing(handle_mid);
        if (nret == MV_OK) {
            std::cout << "🟢开始取流成功"<< std::endl;
        }
        else {
            std::cout << "🔴开始取流失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }

    }

    //取最新的一帧的回调函数
    void __stdcall hik_camera::Capture_Latest_Frame(unsigned char* pdata,
                                        MV_FRAME_OUT_INFO_EX* pframeinfo,
                                        void* puser) {
        auto* camera = static_cast<hik_camera*>(puser);
        {
            std::lock_guard<std::mutex> lock(camera->image_mutex);
            //std::cout << "🟢停止该帧的本机时间戳为 " << pframeinfo->nHostTimeStamp <<",该帧的帧号为 " << pframeinfo->nFrameNum << std::endl;
            camera -> frame.new_recover = true;
            camera -> frame.nhosttimestamp = pframeinfo->nHostTimeStamp;
            camera -> frame.nframenum = pframeinfo->nFrameNum;
            cv::Mat this_image(pframeinfo->nHeight, pframeinfo->nWidth, CV_8UC1, pdata);
            camera -> frame.latest_image = this_image.clone();
        }
        camera -> frame.image_cv.notify_one();
    }

    void hik_camera::Register_Image_Callback() {

        int nret = MV_CC_RegisterImageCallBackEx(handle_mid,&Capture_Latest_Frame, this);
        if (nret != MV_OK) {
            std::cerr << "🔴注册回调失败：" << nret << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Stop_Grabbing() {
        int nret;
        nret = MV_CC_StopGrabbing(handle_mid);
        if (nret == MV_OK) {
            std::cout << "🟢停止取流成功"<< std::endl;
        }
        else {
            std::cout << "🔴停止取流失败，状态码是：" << nret << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    hik_camera::~hik_camera() {
        Stop_Grabbing();
        //camera.Unregister_Buffer();
        //camera.Free_AllocAlignedBuffer();
        Close_Camera();
        Destroy_Camera_Instance();
        std::cout << "🟢第" << devcice_number <<"台相机成功终止"<< std::endl;
    }
}
