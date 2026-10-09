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

namespace nexte {

    std::string hik_camera::Get_SDK_Version(void) {
        std::ostringstream version_sdk_ostringstream;
        version_sdk_ostringstream << version_sdk_major <<"."<< version_sdk_minor <<"."<< version_sdk_revision <<"."<< version_sdk_build;
        std::string version_sdk_string = version_sdk_ostringstream.str();
        return version_sdk_string;
    }

    int hik_camera::SDK_Init(void) {
        int nRet;
        nRet = MV_CC_Initialize();
        if (nRet == MV_OK) {
            std::cout << "🟢SDK成功初始化" << std::endl;
        }
        else {
            std::cout << "🔴SDK失败初始化，状态码是："<< nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        return nRet;
    }

    int hik_camera::SDK_Final(void) {
        int nRet;
        nRet = MV_CC_Finalize();
        if (nRet == MV_OK) {
            std::cout << "🟢SDK成功终止" << std::endl;
        }
        else {
            std::cout << "🔴SDK失败终止,状态码是："<< nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        return nRet;
    }

    MV_CC_DEVICE_INFO_LIST hik_camera::Enumerate_Devices(void) {
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
            std::cout << "🔴设备枚举失败,状态码是："<< nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
        stDeviceList_mid = stDeviceList;
        return stDeviceList;
    }

    void hik_camera::Create_Camera_Instance(unsigned int nIndex) {
        int nRet;
        void* handle = NULL;
        nRet = MV_CC_CreateHandle(&handle, stDeviceList_mid.pDeviceInfo[nIndex]);
        if (nRet == MV_OK) {
            std::cout << "🟢句柄初始化成功，初始化的是第"<< nIndex+1 << "台相机" << std::endl;
            handle_mid = handle;
        }
        else {
            std::cout << "🔴句柄初始化失败,状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Destroy_Camera_Instance() {
        int nRet;
        void* handle = NULL;
        nRet = MV_CC_DestroyHandle(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢句柄销毁成功" << std::endl;
            handle_mid = handle;
        }
        else {
            std::cout << "🔴句柄销毁失败,状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Open_Camera(void) {
        int nRet;
        nRet = MV_CC_OpenDevice(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢成功打开相机" << std::endl;
        }
        else {
            std::cout << "🔴打开相机失败，,状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void hik_camera::Close_Camera(void) {
        int nRet;
        nRet = MV_CC_CloseDevice(handle_mid);
        if (nRet == MV_OK) {
            std::cout << "🟢成功关闭相机" << std::endl;
        }
        else {
            std::cout << "🔴关闭相机失败，,状态码是：" << nRet << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    

    

}
