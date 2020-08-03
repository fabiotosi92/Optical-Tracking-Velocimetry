#include <opencv2/opencv.hpp>
#include "utils.h"
#include "ops.h"

/****************************
/* Video writer
/****************************/

void writeVideo(cv::VideoCapture &capture, float resolution,
					  std::vector<std::vector<cv::KeyPoint>> &keypoints_mem_current, 
	                  std::vector<std::vector<cv::KeyPoint>> &keypoints_mem_predicted, 
	                  std::vector<std::vector<bool>> &valid,
	                  std::vector<std::vector<float>> velocity,
	                  std::vector<std::vector<float>> angle, 
	                  std::vector<std::vector<float>> distance,
	                  float max_final, float min_final,
	                  std::vector<std::vector<float>> &velocity_mem,
	                  std::string output_path);

/****************************
/* File writer
/****************************/

void writeStats(std::string output_path, std::string summary, 
	            std::vector<std::vector<float>> velocity, 
	            std::vector<std::vector<float>> angle, std::vector<std::vector<float>> distance,
	            float avg, float max, float min, float std_dev, float computation_time);
void writeSubRegionStats(std::vector<float>& averages, std::vector<float>& trajectories, 
	                     std::vector<float>& std, int width, int step, std::string output_path);
