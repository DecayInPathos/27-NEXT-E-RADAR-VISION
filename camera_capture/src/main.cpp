/*
 *  _   _  _______   _______   _____
 * | \ | ||  ___\ \ / /_   _| |  ___|
 * |  \| || |__  \ V /  | |   | |__
 * | . ` ||  __| /   \  | |   |  __|
 * | |\  || |___/ /^\ \ | |   | |___
 * \_| \_/\____/\/   \/ \_/   \____/
 *
 * @Author: pathos (王家辉)
 * @Date: 2026-10-08 03:52
 * @Description:
 *
 * Copyright (c) 2026 by XAUT NEXT-E/pathos.
 */



#include "hik_camera.h"
#include <iostream>
#include <sstream>
#include <string>

#include "../include/hik_camera.h"

int main()
{
    nexte::hik_camera camera;
    camera.SDK_Init();
    camera.Get_SDK_Version();
    camera.Enumerate_Devices();
    camera.Create_Camera_Instance(0);
    camera.Open_Camera();
    camera.Set_Exposure(380.0000);
    camera.Set_Gain(0);
    camera.Set_Pixel_Format();
    camera.Set_Frame_Rate(150.0000);
    camera.Open_AcquisitionMode_Mode();
    camera.Get_PayloadSize_Alignment();
    camera.Get_AllocAlignedBuffer();
    camera.Register_Buffer();
    camera.Start_Grabbing();



     camera.Stop_Grabbing();
    camera.Unregister_Buffer();
    camera.Free_AllocAlignedBuffer();
    camera.Close_Camera();
    camera.Destroy_Camera_Instance();
    camera.SDK_Final();
    return 0;
}
