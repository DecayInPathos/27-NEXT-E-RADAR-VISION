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
#include <thread>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <chrono>
#include <thread>

int main()
{
    int nret;
    nret = MV_CC_Initialize();
    if (nret == MV_OK) {
        std::cout << "🟢SDK成功初始化" << std::endl;
    }
    else {
        std::cout << "🔴SDK失败初始化，状态码是："<< nret << std::endl;
        std::exit(EXIT_FAILURE);
    }

    {
        nexte::hik_camera camera0(0);
        while (true) {
            cv::Mat image;
            {
                std::lock_guard<std::mutex> lock(camera0.image_mutex);
                if (!camera0.frame.latest_image.empty() && camera0.frame.is_copy==false) {
                    image = camera0.frame.latest_image.clone();
                    camera0.frame.is_copy = true;
                    std::cout << "🟢停止该帧的本机时间戳为 " << camera0.frame.nhosttimestamp <<",该帧的帧号为 " << camera0.frame.nframenum << std::endl;
                }
                camera0.frame.image_cv.notify_one();
                //start*    图片的处理逻辑





                //begin*    图片的处理逻辑
            }

            if (!image.empty()) {
                cv::Mat bgr_image;
                cv::cvtColor(image, bgr_image, cv::COLOR_BayerGR2BGR);
                //这个打印的过程7ms左右，如果不是平时测试最好不要开启
                cv::imshow("Camera", bgr_image);
            }

            //这里的bgr_image是最终取出且转换为格式的最新一帧
            //start*    图片的处理逻辑

            std::this_thread::sleep_for(std::chrono::milliseconds(100));



            //begin*    图片的处理逻辑
            camera0.frame.is_processed = true;
            camera0.frame.image_cv.notify_one();
            const int key = cv::waitKey(1);
            if (key == 'q' || key == 27) break;
        }

        cv::destroyAllWindows();
        //nexte::hik_camera camera1(1);
    }


    nret = MV_CC_Finalize();
    if (nret == MV_OK) {
        std::cout << "🟢SDK成功终止" << std::endl;
    }
    else {
        std::cout << "🔴SDK失败终止,状态码是："<< nret << std::endl;
        std::exit(EXIT_FAILURE);
    }



}

