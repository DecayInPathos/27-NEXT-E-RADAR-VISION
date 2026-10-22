#pragma once
#include <opencv2/opencv.hpp>
#include <string>

namespace nexte {
	class camera_calibration {
	private:
		std::string  camera_calibration_xml;
		cv::Mat K;
		cv::Mat D;
		int width = 0;
		int height = 0;
		cv::Mat map1, map2;

	public:
		cv::Mat corrected;
		
		camera_calibration(std::string camera_calibration_xml);

		cv::Mat image_corrected(const cv::Mat& raw_image);
	};
}