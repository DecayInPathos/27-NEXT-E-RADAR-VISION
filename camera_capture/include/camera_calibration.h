#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#inlcude <string>
namespace nexte {
	class camera_calibration {
	private:
		std::string const& camera_calibration_xml_adress;
	public:
		camera_calibration(std::string const& camera_calibration_matrix_);
	};
}