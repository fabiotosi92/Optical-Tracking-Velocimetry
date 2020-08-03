#include <opencv2/opencv.hpp>
#include "utils.h"
#include "ops.h"

/****************************
/* File writer
/****************************/

void writeStats(std::string output_path,int count, float avg, float max, float min, float std_dev, float computation_time);
void writeSubRegionStats(std::vector<float>& averages, std::vector<float>& trajectories, 
	                     std::vector<float>& std, int width, int step, std::string output_path);
