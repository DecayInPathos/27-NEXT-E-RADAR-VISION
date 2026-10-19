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
#include <thread>

#include "../include/hik_camera.h"

int main()
{

    unsigned int aa = std::thread::hardware_concurrency();
    std::cout << aa << std::endl;

    nexte::hik_camera camera(0);
    camera.Set_Exposure(380.0000);
    camera.Set_Gain(0);
    camera.Set_Pixel_Format();
    camera.Set_Frame_Rate(150.0000);
    camera.Open_AcquisitionMode_Mode();
    //camera.Get_PayloadSize_Alignment();
    //camera.Get_AllocAlignedBuffer();
    //camera.Register_Buffer();
    camera.Set_Image_Node_Number();
    camera.register_image_callback();
    camera.Start_Grabbing();

    char input;
    do
    {
        std::cin >> input;
        // body of the loop;
    }
    while(input != 'q');

}
