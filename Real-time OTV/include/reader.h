#include <opencv2/opencv.hpp>
#include <fstream>
#include <utils.h>

/****************************
/* File reader
/****************************/

void readCalibration(std::string file_name, float &pixel_to_real, 
	float &partial_min_angle, float &partial_max_angle,
	float &final_min_angle, float &final_max_angle, float &final_min_distance, 
    float &max_features, float &resolution, float &region_step, std::string &feature_detector);

//lukas-kanade parameters
void readLKParameters(std::string file_name, int &radius, 
	                  int &maxLevel, int &maxCount, double &epsilon, 
	                  int &flags, double &minEigThreshold);
	                  
//info-video parameters
void readInfoVideo(std::string file_name, double& fps,
				   int& height, int& width, int& total_frames,
				   std::string& directory_frame, int& jump);
