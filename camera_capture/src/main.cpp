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

#include <cstdio>
#include "MvCameraControl.h"

int main()
{
    std::printf("SDK version: 0x%08X\n", MV_CC_GetSDKVersion());
    return 0;
}