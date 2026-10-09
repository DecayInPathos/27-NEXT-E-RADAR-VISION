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
    std::string version = camera.Get_SDK_Version();
    std::cout << "🟢当前SDK的版本是：" <<version << std::endl;
    camera.Enumerate_Devices();
    camera.Create_Camera_Instance(0);
    camera.Open_Camera();
    
    camera.Close_Camera();
    camera.Destroy_Camera_Instance();
    camera.SDK_Final();
    return 0;
}
