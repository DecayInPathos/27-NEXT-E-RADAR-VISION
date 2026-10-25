/*
*  _   _  _______   _______   _____
 * | \ | ||  ___\ \ / /_   _| |  ___|
 * |  \| || |__  \ V /  | |   | |__
 * | . ` ||  __| /   \  | |   |  __|
 * | |\  || |___/ /^\ \ | |   | |___
 * \_| \_/\____/\/   \/ \_/   \____/
 *
 * @Author: pathos (王家辉)
 * @Date: 2026-10-20 01:37
 * @Description: 相机标定参数读取与图像去畸变
 *
 * Copyright (c) 2026 by XAUT NEXT-E/pathos.
 */
#include "camera_calibration.h"
namespace nexte {
	camera_calibration::camera_calibration(std::string  camera_calibration_xml_)
	:camera_calibration_xml(camera_calibration_xml_) {
		cv::FileStorage fs(
		camera_calibration_xml_,
		cv::FileStorage::READ);

		if (!fs.isOpened()) {
			std::cerr << "🔴无法打开标定文件\n";
			std::exit(EXIT_FAILURE);
		}

		fs["camera_matrix"] >> K;
		fs["distortion_coefficients"] >> D;
		fs["image_width"] >> width;
		fs["image_height"] >> height;
		fs.release();

		cv::initUndistortRectifyMap(
	K, D,
	 cv::Mat(),                // 不额外旋转图像
	 K,                        // 矫正后图像使用的内参
	    cv::Size(width, height),   // 图像尺寸
	    CV_16SC2,                 // 映射表的存储格式
	 map1, map2                // 输出两张映射表
);
	}


	cv::Mat camera_calibration::image_corrected(const cv::Mat& raw_image)
	{
		if (raw_image.empty()) {
			return cv::Mat();
		}

		CV_Assert(raw_image.size() == cv::Size(width, height));

		cv::remap(
			raw_image,
			corrected,
			map1, map2,
			cv::INTER_LINEAR
		);

		return corrected;
	}
}